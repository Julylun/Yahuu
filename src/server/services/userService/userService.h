#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <stdbool.h> // For bool type
#include <stdlib.h>  // For long type

/**
 * @brief Ensures the necessary 'user' collection exists in the database.
 * This function should be called once on server startup to initialize
 * the data structure for users.
 * @return 0 on success or if the collection already exists, -1 on critical failure.
 */
int UserService_createDocument();

/**
 * @brief Registers a new user in the system.
 * Checks for username uniqueness, (should hash the password), and creates the user.
 * @param username The desired username. Must be unique.
 * @param password The user's plaintext password.
 * @return The new user's ID on success, -1 on failure (e.g., username already exists).
 */
long UserService_register(const char* username, const char* password);

#include "../../models/usermodel/userModel.h" // For User struct

/**
 * @brief Authenticates a user and returns their data on success.
 * Verifies the given username and password against the database.
 * @param username The user's username.
 * @param password The user's plaintext password.
 * @return A pointer to a User struct on successful login, NULL otherwise.
 *         The caller is responsible for freeing the returned User struct using User_free().
 */
User* UserService_login(const char* username, const char* password);

/**
 * @brief Deletes a user from the system by their ID.
 * @param id The ID of the user to delete.
 * @return 0 on success, -1 on failure (e.g., user not found).
 */
int UserService_delete(long id);


#endif // USER_SERVICE_H
