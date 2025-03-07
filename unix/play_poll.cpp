#include <sys/poll.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    while (true) {
        int ret = poll(fds, 1, 3000);
        if (ret == -1) {
            perror("poll");
            exit(1);
        }
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                char buf[1024];
                ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
                if (len == -1) {
                    perror("read");
                    exit(1);
                }
                if (len == 0) {
                    break;
                }
                if (write(STDOUT_FILENO, buf, len) == -1) {
                    perror("write");
                    exit(1);
                }
            }
        } else {
            perror("timeout");
        }
    }
}