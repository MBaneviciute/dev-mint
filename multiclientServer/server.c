#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 10

void *client_handler(void *);

int main(int argc, char *argv[]) {
    int server_socket, client_socket, c, *new_sock;
    struct sockaddr_in server, client;
    pthread_t thread_id[MAX_CLIENTS];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // Bind the socket to specified IP and port
    if (bind(server_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(server_socket, MAX_CLIENTS);

    // Accept incoming connections
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *) &client, (socklen_t *) &c))) {
        puts("Connection accepted");

        // Create a new thread to handle the client
        new_sock = malloc(1);
        *new_sock = client_socket;
        if (pthread_create(&thread_id[MAX_CLIENTS], NULL, client_handler, (void *) new_sock) < 0) {
            perror("Could not create thread");
            exit(EXIT_FAILURE);
        }

        // Join the thread when it's done
        pthread_join(thread_id[MAX_CLIENTS], NULL);
        puts("Handler assigned");
    }

    if (client_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void *client_handler(void *socket_desc) {
    // Get the socket descriptor
    int client_socket = *(int *) socket_desc;
    int read_size;
    char *message, client_message[2000];

    // Send welcome message to the client
    message = "Welcome to the server!\n";
    write(client_socket, message, strlen(message));

    // Receive message from client
    while ((read_size = recv(client_socket, client_message, 2000, 0)) > 0) {
        // Add null terminator to the end of the message
        client_message[read_size] = '\0';

        // Send the message back to the client
        write(client_socket, client_message, strlen(client_message));

        // Clear the buffer
        memset(client_message, 0, 2000);
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    // Free the socket descriptor
    free(socket_desc);

    return NULL;
}
