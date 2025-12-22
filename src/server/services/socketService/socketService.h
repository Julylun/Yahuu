#ifndef SOCKET_SERVICE_H
#define SOCKET_SERVICE_H

#include <stdio.h>
#include <stdlib.h>

// Headers for socket programming on Linux/POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // For close()

/**
 * @brief Initializes a new server socket.
 * Creates a TCP socket, configures it to reuse address/port, binds it 
 * to the specified port on all available network interfaces, and puts 
 * it into a listening state.
 *
 * @param port The port number to listen on.
 * @param max_connections The maximum number of pending connections in the queue (backlog).
 * @return The file descriptor for the listening server socket on success.
 * @return -1 on failure (and prints error details to stderr).
 */
int Socket_init(int port, int max_connections);

/**
 * @brief Starts the main server loop to accept and handle incoming connections.
 * This function will block indefinitely.
 * @param server_fd The file descriptor of the listening server socket.
 */
void Socket_run(int server_fd);

#endif // SOCKET_SERVICE_H
