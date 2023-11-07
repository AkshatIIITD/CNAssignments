#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define PORT 8080

unsigned long long fact(unsigned long long n) {                                // Function to compute the factorial
    if (n <= 1) {
        return 1;
    }
    return n * fact(n - 1);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {                  // Create a socket
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {   // Bind the socket    
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {                                            // Listen for incoming connections
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        int pid = fork();                                      // Child process
        if (pid == 0) {
            close(server_fd);                                  // Close the listening socket in the child
            unsigned long long n;                              // Read the payload from the client
            read(new_socket, &n, sizeof(n));
            if (n > 20) {                                      // Compute the factorial (cap at 20 if n > 20)
                n = 20;
            }
            unsigned long long result = fact(n);
            write(new_socket, &result, sizeof(result));
            close(new_socket);                                 // Close the client socket
            exit(0);
        } else if (pid < 0) {
            perror("Fork");
            exit(EXIT_FAILURE);
        }
        close(new_socket);                                     // Close the client socket in the parent
    }

    return 0;
}
