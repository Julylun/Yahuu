#include "networkService.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

// POSIX/Linux headers for networking
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// --- Module-level static variables ---
static int g_socket_fd = -1;
static pthread_t g_listener_thread;
static bool g_is_connected = false;

// Buffer for the last server response and a mutex to protect it
static char g_last_response[2048];
static pthread_mutex_t g_response_mutex;

// Callback function for asynchronous, server-pushed messages
static async_message_handler_t g_async_handler = NULL;



// The function that will run in the background to handle incoming messages
static void* handleConnection(void* arg) {
    char server_message[2048];
    const char* separator = "^";
    
    while (g_is_connected) {
        memset(server_message, 0, sizeof(server_message));
        int recv_size = recv(g_socket_fd, server_message, sizeof(server_message) - 1, 0);
        
        if (recv_size > 0) {
            server_message[recv_size] = '\0';
            printf("[Server Response]: %s\n", server_message);

            // Make a copy to safely parse the command
            char* msg_copy = strdup(server_message);
            if (msg_copy == NULL) { continue; } // strdup failed
            char* command = strtok(msg_copy, separator);

            bool is_async = false;
            if(command != NULL) {
                // Add any future async commands here
                if (strcmp(command, "RECEIVE_DM") == 0 || strcmp(command, "RECEIVE_GROUP_MSG") == 0) {
                    is_async = true;
                }
            }
            free(msg_copy);

            if (is_async && g_async_handler != NULL) {
                // This is a pushed message from the server, use the callback
                g_async_handler(server_message);
            } else {
                // This is a direct reply to a client command, store it in the buffer
                pthread_mutex_lock(&g_response_mutex);
                strncpy(g_last_response, server_message, sizeof(g_last_response) - 1);
                g_last_response[sizeof(g_last_response) - 1] = '\0'; // Ensure null-termination
                pthread_mutex_unlock(&g_response_mutex);
            }

        } else {
            // Server closed connection or an error occurred
            if (recv_size == 0) printf("Connection closed by server.\n");
            else perror("recv failed");
            g_is_connected = false;
        }
    }
    
    printf("Listener thread finished.\n");
    return NULL;
}

int Network_connect(const char* ip, int port) {
    if (g_is_connected) return g_socket_fd;

    // Initialize the response mutex
    if (pthread_mutex_init(&g_response_mutex, NULL) != 0) {
        perror("mutex init failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    g_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socket_fd == -1) {
        perror("Could not create socket");
        return -1;
    }

    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (connect(g_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(g_socket_fd);
        g_socket_fd = -1;
        return -1;
    }

    printf("Connected to server at %s:%d\n", ip, port);
    g_is_connected = true;
    Network_clear_response(); // Clear any old responses
    return g_socket_fd;
}

int Network_start_listener() {
    if (!g_is_connected) return -1;
    if (pthread_create(&g_listener_thread, NULL, handleConnection, NULL) != 0) {
        perror("could not create listener thread");
        return -1;
    }
    printf("Network listener thread started.\n");
    return 0;
}

int Network_send(const char* message) {
    if (!g_is_connected) return -1;
    if (send(g_socket_fd, message, strlen(message), 0) < 0) {
        perror("Send failed");
        return -1;
    }
    return 0;
}

void Network_disconnect() {
    if (g_is_connected) {
        g_is_connected = false;
        // Shutting down the socket for reading will cause recv() in the listener
        // thread to return immediately, allowing the thread to exit gracefully.
        shutdown(g_socket_fd, SHUT_RD); 
        
        pthread_join(g_listener_thread, NULL); // Wait for the listener thread
        close(g_socket_fd);
        g_socket_fd = -1;

        pthread_mutex_destroy(&g_response_mutex); // Clean up the mutex
        printf("Disconnected from server.\n");
    }
}

void Network_clear_response() {
    pthread_mutex_lock(&g_response_mutex);
    memset(g_last_response, 0, sizeof(g_last_response));
    pthread_mutex_unlock(&g_response_mutex);
}

void Network_set_async_message_handler(async_message_handler_t handler) {
    g_async_handler = handler;
}

void Network_get_response(char* buffer, int buffer_size) {
    pthread_mutex_lock(&g_response_mutex);
    strncpy(buffer, g_last_response, buffer_size - 1);
    buffer[buffer_size - 1] = '\0'; // Ensure null termination
    pthread_mutex_unlock(&g_response_mutex);
}
