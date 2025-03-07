#include <liburing.h>
#include <liburing/io_uring.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <string.h>

int main() {
    constexpr const unsigned BUFFER_SIZE = 4096;
    struct io_uring ring;
    int ret;

    ret = io_uring_queue_init(8, &ring, 0);
    if (ret < 0) {
        perror("io_uring_queue_init");
        return 1;
    }

    int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char buffer[BUFFER_SIZE] = "Hello, World!";
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    struct io_uring_cqe *cqe;
    if (!sqe) {
        perror("io_uring_get_sqe");
        return 1;
    }

    sqe->user_data = 338;
    io_uring_prep_write(sqe, fd, buffer, strlen(buffer) - 1, 1);
    
    ret = io_uring_submit(&ring);
    if (ret < 0) {
        perror("io_uring_submit");
        return 1;
    }

    io_uring_wait_cqe(&ring, &cqe);

    printf("[%d] Write completed with result %d\n", cqe->user_data, cqe->res);

    io_uring_queue_exit(&ring);
    close(fd);

}