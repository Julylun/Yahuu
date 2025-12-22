#include "socketService.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "socketService.h"
#include "../userService/userService.h" // Include the user service to handle logic
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// This is the new, command-aware client handler
static void* client_handler(void* socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);

    char client_message[2048];
    char response[2048];
    int read_size;

    // Define the separator for parsing commands
    const char* separator = "^";

    while ((read_size = recv(sock, client_message, sizeof(client_message) - 1, 0)) > 0) {
        client_message[read_size] = '\0';
        printf("Received from client %d: %s\n", sock, client_message);

        // Make a copy for strtok, as it modifies the string
        char* msg_copy = strdup(client_message);
        char* command = strtok(msg_copy, separator);

        if (command == NULL) {
            snprintf(response, sizeof(response), "ERROR^INVALID_COMMAND_FORMAT");
        } else if (strcmp(command, "REGISTER") == 0) {
            char* username = strtok(NULL, separator);
            char* password = strtok(NULL, separator);

            if (username && password) {
                long new_id = UserService_register(username, password);
                if (new_id > 0) {
                    snprintf(response, sizeof(response), "REGISTER_SUCCESS^%ld", new_id);
                } else {
                    snprintf(response, sizeof(response), "REGISTER_FAIL^USERNAME_TAKEN_OR_INVALID");
                }
            } else {
                snprintf(response, sizeof(response), "REGISTER_FAIL^INSUFFICIENT_ARGS");
            }
        } else if (strcmp(command, "LOGIN") == 0) {
            char* username = strtok(NULL, separator);
            char* password = strtok(NULL, separator);

            if (username && password) {
                if (UserService_login(username, password)) {
                    snprintf(response, sizeof(response), "LOGIN_SUCCESS");
                } else {
                    snprintf(response, sizeof(response), "LOGIN_FAIL^INVALID_CREDENTIALS");
                }
            } else {
                snprintf(response, sizeof(response), "LOGIN_FAIL^INSUFFICIENT_ARGS");
            }
        } else {
            snprintf(response, sizeof(response), "ERROR^UNKNOWN_COMMAND");
        }
        
        free(msg_copy);

        // Send the response back to the client
        printf("Sending response to client %d: %s\n", sock, response);
        write(sock, response, strlen(response));
        
        memset(client_message, 0, sizeof(client_message));
        memset(response, 0, sizeof(response));
    }

    if (read_size == 0) {
        printf("Client %d disconnected.\n", sock);
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    close(sock);
    return 0;
}


int Socket_init(int port, int max_connections) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // 1. Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    // 2. Set socket options to allow reusing the address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_fd);
        return -1;
    }

    // Initialize sockaddr_in structure
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 3. Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return -1;
    }

    // 4. Put the server socket in a passive mode to wait for clients
    if (listen(server_fd, max_connections) < 0) {
        perror("listen failed");
        close(server_fd);
        return -1;
    }

    printf("Server socket initialized. Listening on port %d\n", port);
    return server_fd;
}

void Socket_run(int server_fd) {
    printf("Server is running. Waiting for incoming connections...\n");

    struct sockaddr_in client_addr;
    int c = sizeof(struct sockaddr_in);
    int client_sock;

    while ((client_sock = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&c))) {
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }
        
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_t client_thread;
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&client_thread, NULL, client_handler, (void*)new_sock) < 0) {
            perror("could not create thread");
            free(new_sock);
            close(client_sock);
        } else {
            pthread_detach(client_thread);
            printf("Handler thread assigned for client %d.\n", client_sock);
        }
    }
    
    // If the loop exits, it's a critical failure.
    if (client_sock < 0) {
        perror("accept failed, shutting down server");
        close(server_fd);
    }
}
