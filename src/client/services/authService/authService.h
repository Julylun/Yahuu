#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <stdbool.h>

/**
 * @brief Attempts to register a new user with the server.
 * This is a blocking call that sends the request and waits for a
 * confirmation from the server.
 * @param username The desired username.
 * @param password The desired password.
 * @return true on successful registration, false otherwise (e.g., user exists, server error).
 */
bool AuthService_register(const char* username, const char* password);

/**
 * @brief Attempts to log in a user.
 * This is a blocking call that sends the credentials and waits for a
 * confirmation from the server.
 * @param username The user's username.
 * @param password The user's password.
 * @return true on successful login, false otherwise (e.g., wrong credentials).
 */
bool AuthService_login(const char* username, const char* password);

#endif // AUTH_SERVICE_H
