#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>


#define MAX_CLIENTS 5

int main(int argc, char *argv[]) 
{
    int listen_sock, client_sock[MAX_CLIENTS], max_sd, activity, i, valread;
    struct sockaddr_in server, client;
    char buffer[1024] = {0};
    fd_set readfds;

    // Create socket
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) 
    {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    // Bind the socket to a local address
    if (bind(listen_sock, (struct sockaddr *) &server, sizeof(server)) < 0) 
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(listen_sock, 3);

    puts("Waiting for incoming connections...");

    // Initialize client sockets array to 0
    for (i = 0; i < MAX_CLIENTS; i++) 
    {
        client_sock[i] = 0;
    }

    while (1) 
    {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add listen socket to the set
        FD_SET(listen_sock, &readfds);
        max_sd = listen_sock;

        // Add child sockets to the set
        for (i = 0; i < MAX_CLIENTS; i++) 
        {
            int sd = client_sock[i];

            // If valid socket descriptor then add to read list
            if (sd > 0) 
            {
                FD_SET(sd, &readfds);
            }

            // Highest file descriptor number, need it for select function
            if (sd > max_sd) 
            {
                max_sd = sd;
            }
        }

        // Wait for activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) 
        {
            perror("Select failed");
            exit(EXIT_FAILURE);
        }

        // If activity on listen socket, it's an incoming connection
        if (FD_ISSET(listen_sock, &readfds)) 
        {
            int c = sizeof(struct sockaddr_in);
            client_sock[MAX_CLIENTS] = accept(listen_sock, (struct sockaddr *) &client, (socklen_t*) &c);
            if (client_sock[MAX_CLIENTS] < 0) 
            {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }
            puts("New client connected");

            // Add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) 
            {
                if (client_sock[i] == 0) {
                    client_sock[i] = client_sock[MAX_CLIENTS];
                    break;
                }
            }
        }

        // Check if any client socket has activity
        for (i = 0; i < MAX_CLIENTS; i++) 
        {
            int sd = client_sock[i];

    if (FD_ISSET(sd, &readfds)) 
    {
        // Check if it was for closing
        if ((valread = read(sd, buffer, 1024)) == 0) 
        {
            // Client disconnected
            close(sd);
            client_sock[i] = 0;
            printf("Client %d disconnected\n", i + 1);
        } 
        else 
        {
            // Echo back the message that came in
            buffer[valread] = '\0';
            send(sd, buffer, strlen(buffer), 0);
        }
        
    }
        }
        }
}