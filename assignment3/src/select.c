#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
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
    fd_set readfds;
    int client_sockets[MAX_CLIENTS];
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
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
                if (sd > max_sd) {
                    max_sd = sd;
                }
            }
        }
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(server_fd, &readfds)) {               // Handling new connection
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                char buffer[1024];
                int valread = read(sd, buffer, sizeof(buffer));
                if (valread <= 0) {
                    close(sd);                                       // Client disconnected or error
                    client_sockets[i] = 0;
                } else {
                    unsigned long long n;
                    memcpy(&n, buffer, sizeof(unsigned long long));  // Processing data and sending response
                    if (n > 20) {                                    // Computing the factorial (cap at 20 if n > 20)
                        n = 20;
                    }
                    unsigned long long result = fact(n);
                    write(sd, &result, sizeof(result));              // Sending the factorial result to the client
                }
            }
        }
    }

    return 0;
}
