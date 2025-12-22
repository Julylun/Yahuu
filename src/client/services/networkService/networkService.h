#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

/**
 * @brief Attempts to connect to the server.
 * @param ip The IP address of the server (e.g., "127.0.0.1").
 * @param port The port number of the server.
 * @return The socket file descriptor on success, or -1 on failure.
 */
int Network_connect(const char* ip, int port);

/**
 * @brief Starts the background thread to listen for incoming server messages.
 * The handleConnection function will run inside this thread.
 * @param socket_fd The socket file descriptor from a successful connection.
 * @return 0 on success, -1 on thread creation failure.
 */
int Network_start_listener();

/**
 * @brief Sends a message to the server.
 * This function should be thread-safe.
 * @param message The null-terminated string to send.
 * @return 0 on success, -1 on failure (e.g., not connected).
 */
int Network_send(const char* message);

/**
 * @brief Disconnects from the server and cleans up the networking thread.
 */
void Network_disconnect();

/**
 * @brief Clears the last received server response buffer.
 * Should be called before sending a new command that expects a response.
 */
void Network_clear_response();

/**
 * @brief Copies the last received server response into a provided buffer.
 * This is the thread-safe way for other services to get results from the network thread.
 * @param buffer The buffer to copy the response into.
 * @param buffer_size The size of the provided buffer.
 */
void Network_get_response(char* buffer, int buffer_size);

#endif // NETWORK_SERVICE_H
