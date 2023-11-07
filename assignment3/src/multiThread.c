#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define PORT 8080

unsigned long long fact(unsigned long long n) {           // Function to compute the factorial
    if (n <= 1) {
        return 1;
    }
    return n * fact(n - 1);
}

void *client_handler(void *arg) {                         // Function to handle client connections in separate threads
    int new_socket = *((int *)arg);
    unsigned long long n;
    read(new_socket, &n, sizeof(n));
    if (n > 20) {                                         // Computing the factorial (cap at 20 if n > 20)
        n = 20;
    }
    unsigned long long result = fact(n);
    write(new_socket, &result, sizeof(result));
    close(new_socket);
    pthread_exit(NULL);
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
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        } 
        pthread_t thread;                     // Creating a new thread to handle the client
        if (pthread_create(&thread, NULL, client_handler, &new_socket) != 0) {
            perror("Thread creation");
            exit(EXIT_FAILURE);
        }
        pthread_detach(thread);               // Detaching the thread to allow it to clean up automatically
    }
    return 0;
}
