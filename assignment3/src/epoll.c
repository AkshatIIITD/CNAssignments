#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#define PORT 8080
#define MAX_EVENTS 4000

unsigned long long fact(unsigned long long n) {           // Function to compute the factorial
    if (n <= 1) {
        return 1;
    }
    return n * fact(n - 1);
}
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {                     // Listening for incoming connections
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    int epfd = epoll_create1(0);                        // Create and configure an epoll instance
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }
    while (1) {
        int num_events = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                    perror("Accept");
                    exit(EXIT_FAILURE);
                }
                ev.events = EPOLLIN;
                ev.data.fd = new_socket;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, new_socket, &ev) == -1) {
                    perror("epoll_ctl: new_socket");
                    exit(EXIT_FAILURE);
                }
            } else {
                char buffer[1024];
                int valread = read(events[i].data.fd, buffer, sizeof(buffer));
                if (valread <= 0) {
                    close(events[i].data.fd);
                } else {
                    unsigned long long n;
                    memcpy(&n, buffer, sizeof(unsigned long long));
                    if (n > 20) {
                        n = 20;
                    }
                    unsigned long long result = fact(n);
                    write(events[i].data.fd, &result, sizeof(result));
                }
            }
        }
    }
    return 0;
}
