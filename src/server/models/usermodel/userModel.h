#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <stdlib.h> // For size_t, long
#include <stdbool.h> // For bool

// Represents a User object.
// In a real application, the password should be a securely stored hash.
typedef struct {
    long id;
    char* username;
    char* password;
} User;

/**
 * @brief Creates a new user record in the database.
 * @param user_data A pointer to a User struct containing username and password.
 *                  The 'id' field is ignored as a new one is generated automatically.
 * @return The ID of the newly created user on success, or -1 on failure.
 */
long User_create(const User* user_data);

/**
 * @brief Reads a user from the database by their unique ID.
 * @param id The ID of the user to retrieve.
 * @return A pointer to a dynamically allocated User struct, or NULL if not found/error.
 *         The caller is responsible for freeing the returned struct with User_free().
 */
User* User_read(long id);

/**
 * @brief Reads a user from the database by their unique username.
 * @param username The username of the user to retrieve.
 * @return A pointer to a dynamically allocated User struct, or NULL if not found/error.
 *         The caller is responsible for freeing the returned struct with User_free().
 */
User* User_read_by_username(const char* username);

/**
 * @brief Updates an existing user's record in the database.
 * The user is identified by the ID field in the provided struct.
 * @param user_data A pointer to the User struct with updated data.
 * @return 0 on success, -1 on failure (e.g., user not found).
 */
int User_update(const User* user_data);

/**
 * @brief Deletes a user from the database by their ID.
 * @param id The ID of the user to delete.
 * @return 0 on success, -1 on failure (e.g., user not found).
 */
int User_delete(long id);

/**
 * @brief Frees the memory allocated for a User struct, including its members.
 * @param user The User struct to free.
 */
void User_free(User* user);


#endif // USER_MODEL_H
