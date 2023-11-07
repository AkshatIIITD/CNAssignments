#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#define PORT 8080
#define MAX_CLIENTS 1000

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
    struct pollfd fds[MAX_CLIENTS];
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
    if (listen(server_fd, 3) < 0) {                       // Listening for incoming connections
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    fds[0].fd = server_fd;                                // Initializing the poll structure for the server socket
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    for (int i = 1; i < MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }
    while (1) {
        // Using poll to wait for events on the server socket and client sockets
        int num_fds = poll(fds, MAX_CLIENTS, -1);
        if (num_fds == -1) {
            perror("Poll");
            exit(EXIT_FAILURE);
        }
        if (fds[0].revents & POLLIN) {
            // New client connection
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }
            for (int i = 1; i < MAX_CLIENTS; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = new_socket;
                    fds[i].events = POLLIN;
                    break;
                }
            }
            fds[0].revents = 0;
        }
        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (fds[i].revents & POLLIN) {                      // Reading data, computing factorial, and sending response
                char buffer[1024];        
                int valread = read(fds[i].fd, buffer, sizeof(buffer));
                if (valread <= 0) {
                    close(fds[i].fd);                           // Client disconnected or error
                    fds[i].fd = -1;
                } else {
                    unsigned long long n;                       // Process data and send response
                    memcpy(&n, buffer, sizeof(unsigned long long));
                    if (n > 20) {                               // Computing the factorial (cap at 20 if n > 20)
                        n = 20;
                    }
                    unsigned long long result = fact(n);
                    write(fds[i].fd, &result, sizeof(result));  // Sending the factorial result to the client
                }
                fds[i].revents = 0;
            }
        }
    }
    return 0;
}
