#include <cstddef>
#include <fcntl.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <map>
#include <vector>

int set_socket_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    return -1;
  }
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

struct UserData {
  int fd;
  int type;
  char buffer[4096];
};

// 客户端连接状态管理
struct ClientState {
  std::vector<char> write_buffer; // 待写入数据
  size_t bytes_written = 0;       // 已写入字节数
  bool has_pending_write = false; // 是否有未完成的写操作
};

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <epoll|uring>\n", argv[0]);
    return 1;
  }

  int mode = 0; // epoll
  if (strcmp(argv[1], "epoll") != 0) {
    mode = 1; // uring
  }

  printf("Running in %s mode\n", mode == 0 ? "epoll" : "uring");

  auto server = socket(AF_INET, SOCK_STREAM, 0);
  if (server == -1) {
    perror("socket");
    return 1;
  }

  // make sockaddr with local ip and port 8384
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8384);
  addr.sin_addr.s_addr = INADDR_ANY;
  auto ret = bind(server, (struct sockaddr *)&addr, sizeof(addr));
  set_socket_nonblocking(server);
  if (ret == -1) {
    perror("bind");
    return 1;
  }
  ret = listen(server, 1024);
  if (ret == -1) {
    perror("listen");
    return 1;
  }

  if (mode == 0) {
    auto epoll_fd = epoll_create1(0);
    struct epoll_event event, events[1024];
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = server;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server, &event);
    if (ret == -1) {
      perror("epoll_ctl");
      return 1;
    }

    // 客户端状态映射
    std::map<int, ClientState> client_states;

    while (true) {
      int nfds = epoll_wait(epoll_fd, events, 1024, -1);
      if (nfds == -1) {
        perror("epoll_wait");
        return 1;
      }
      
      for (int i = 0; i < nfds; ++i) {
        if (events[i].data.fd == server) {
          // 循环接受所有连接以处理边缘触发模式
          while (true) {
            auto client = accept(server, nullptr, nullptr);
            if (client == -1) {
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有更多连接请求
                break;
              }
              perror("accept");
              return 1;
            }
                        
            set_socket_nonblocking(client);
            
            // 初始化客户端状态
            client_states[client] = ClientState{};
            
            // 注册客户端读事件
            struct epoll_event client_event;
            client_event.events = EPOLLIN | EPOLLET;
            client_event.data.fd = client;
            ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &client_event);
            if (ret == -1) {
              perror("epoll_ctl");
              return 1;
            }
          }
        } else {
          int client_fd = events[i].data.fd;
          
          // 处理写事件
          if ((events[i].events & EPOLLOUT) && client_states[client_fd].has_pending_write) {
            ClientState& state = client_states[client_fd];
            
            while (state.bytes_written < state.write_buffer.size()) {
              ssize_t result = write(client_fd, 
                                   state.write_buffer.data() + state.bytes_written, 
                                   state.write_buffer.size() - state.bytes_written);
              
              if (result == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                  // 缓冲区已满，等待下一次EPOLLOUT事件
                  break;
                }
                perror("write");
                // 发生错误，关闭连接
                close(client_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                client_states.erase(client_fd);
                goto client_continue; // 跳到下一个事件
              }
              
              state.bytes_written += result;
              printf("write %zd bytes to client %d (total: %zu/%zu)\n", 
                    result, client_fd, state.bytes_written, state.write_buffer.size());
            }
            
            // 所有数据写完，重置状态并更新为只关注读事件
            if (state.bytes_written >= state.write_buffer.size()) {
              state.write_buffer.clear();
              state.bytes_written = 0;
              state.has_pending_write = false;
              
              struct epoll_event client_event;
              client_event.events = EPOLLIN | EPOLLET;
              client_event.data.fd = client_fd;
              epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &client_event);
            }
          }
          
          // 处理读事件
          if (events[i].events & EPOLLIN) {
            char buffer[4096];
            
            // 循环读取所有数据
            while (true) {
              auto len = read(client_fd, buffer, sizeof(buffer));
              if (len == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                  // 没有更多数据可读
                  break;
                }
                perror("read");
                close(client_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                client_states.erase(client_fd);
                goto client_continue;
              }
              
              if (len == 0) {
                // 客户端断开连接
                close(client_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                client_states.erase(client_fd);
                goto client_continue;
              }
              
              
              // 将读取的数据追加到写缓冲区
              ClientState& state = client_states[client_fd];
              size_t old_size = state.write_buffer.size();
              state.write_buffer.resize(old_size + len);
              memcpy(state.write_buffer.data() + old_size, buffer, len);
              state.has_pending_write = true;
              
              // 尝试立即写回数据
              ssize_t write_result = write(client_fd, 
                                         state.write_buffer.data() + state.bytes_written, 
                                         state.write_buffer.size() - state.bytes_written);
              
              if (write_result == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                  // 写缓冲区已满，注册EPOLLOUT事件
                  struct epoll_event client_event;
                  client_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
                  client_event.data.fd = client_fd;
                  epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &client_event);
                  break;
                }
                perror("write");
                close(client_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                client_states.erase(client_fd);
                goto client_continue;
              }
              
              state.bytes_written += write_result;
              
              // 如果写完了所有数据，重置状态
              if (state.bytes_written >= state.write_buffer.size()) {
                state.write_buffer.clear();
                state.bytes_written = 0;
                state.has_pending_write = false;
              } else {
                // 部分数据未写完，注册EPOLLOUT事件
                struct epoll_event client_event;
                client_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
                client_event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &client_event);
                break; // 等待下次写事件
              }
            }
          }
client_continue:
          continue;
        }
      }
    }
  } else {
    struct io_uring ring;
    ret = io_uring_queue_init(1024, &ring, 0);
    if (ret < 0) {
      perror("io_uring_queue_init");
      return 1;
    }
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(sqe, server, NULL, NULL, 0);    
    auto server_data = new UserData{server, 0};
    io_uring_sqe_set_data(sqe, server_data);
    ret = io_uring_submit(&ring);
    if (ret < 0) {
      perror("io_uring_submit");
      return 1;
    }
    while (true) {
        struct io_uring_cqe *cqe;
        ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0) {
            perror("io_uring_wait_cqe");
            return 1;
        }
        auto user_data = (UserData *)io_uring_cqe_get_data(cqe);
        if (user_data->type == 0) {
            int client_socket = cqe->res;
            set_socket_nonblocking(client_socket);
            auto server_sqe = io_uring_get_sqe(&ring);
            io_uring_prep_accept(server_sqe, server, NULL, NULL, 0);
            server_sqe->user_data = cqe->user_data;
            io_uring_submit(&ring);

            auto client_sqe = io_uring_get_sqe(&ring);
            auto client_data = new UserData{client_socket, 1};
            io_uring_prep_read(client_sqe, client_socket, client_data->buffer, sizeof(client_data->buffer), 0);
            client_sqe->user_data = (unsigned long)client_data;
            io_uring_submit(&ring);            
        } else if (user_data->type == 1) { // read done
            auto client_socket = user_data->fd;
            auto len = cqe->res;
            if (len == 0) {
                close(client_socket);
                delete user_data;
            } else {
                auto write_sqe = io_uring_get_sqe(&ring);
                io_uring_prep_write(write_sqe, client_socket, user_data->buffer, len, 0);
                user_data->type = 2;
                write_sqe->user_data = cqe->user_data;
                io_uring_submit(&ring);
            }
        } else { // write done
            auto client_socket = user_data->fd;
            auto len = cqe->res;
            if (len == 0) {
                close(client_socket);
                delete user_data;
            } else {
                auto read_sqe = io_uring_get_sqe(&ring);
                io_uring_prep_read(read_sqe, client_socket, user_data->buffer, sizeof(user_data->buffer), 0);
                user_data->type = 1;
                read_sqe->user_data = cqe->user_data;
                io_uring_submit(&ring);
            }
        }
        io_uring_cqe_seen(&ring, cqe);
    }
  }
}