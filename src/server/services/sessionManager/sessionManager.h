#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

// Represents an active user session
typedef struct {
    long userId;    // The ID of the logged-in user
    int socket_fd;  // The socket file descriptor for the client connection
    char username[256]; // The username of the logged-in user
} UserSession;

/**
 * @brief Initializes the session manager.
 * Must be called once when the server starts.
 */
void SessionManager_init();

/**
 * @brief Adds a new user session to the manager.
 * Should be called after a user successfully logs in.
 * @param userId The ID of the user.
 * @param username The username of the user.
 * @param socket_fd The client's socket file descriptor.
 * @return 0 on success, -1 on failure (e.g., manager is full).
 */
int SessionManager_add(long userId, const char* username, int socket_fd);

/**
 * @brief Removes a user session based on the socket file descriptor.
 * Should be called when a client disconnects.
 * @param socket_fd The socket file descriptor of the disconnected client.
 */
void SessionManager_remove_by_socket(int socket_fd);

/**
 * @brief Finds the socket file descriptor for a given user ID.
 * @param userId The ID of the user to find.
 * @return The socket file descriptor if the user is online, otherwise -1.
 */
int SessionManager_get_socket(long userId);

/**
 * @brief Finds the user ID for a given socket file descriptor.
 * @param socket_fd The socket file descriptor to find.
 * @return The user ID if the session exists, otherwise -1.
 */
long SessionManager_get_user(int socket_fd);

/**
 * @brief Finds the user session for a given socket file descriptor.
 * @param socket_fd The socket file descriptor to find.
 * @return A pointer to the UserSession if found, otherwise NULL.
 */
const UserSession* SessionManager_get_session_by_socket(int socket_fd);

#endif // SESSION_MANAGER_H
