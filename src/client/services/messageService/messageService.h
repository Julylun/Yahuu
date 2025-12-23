#ifndef MESSAGE_SERVICE_H
#define MESSAGE_SERVICE_H

#include <stdbool.h>

/**
 * @brief Sends a direct message to another user.
 * This is a blocking call that sends the message and waits for a
 * confirmation from the server.
 * @param receiverId The ID of the user to send the message to.
 * @param message The content of the message.
 * @return true on successful send and confirmation, false otherwise.
 */
bool MessageService_send_dm(long receiverId, const char* message);

/**
 * @brief Requests the message history with another user.
 * This is a blocking call that sends the request and waits for the
 * full history data from the server.
 * @param contactId The ID of the other user in the conversation.
 * @return A dynamically allocated string containing the history data, or NULL on failure.
 *         The caller is responsible for freeing this string.
 */
char* MessageService_get_history(long contactId);

/**
 * @brief Gets a list of unique user IDs that the current user has conversed with.
 * 
 * @param count A pointer to an integer where the number of contacts will be stored.
 * @return A dynamically allocated array of user IDs, or NULL on failure or if no contacts exist.
 *         The caller is responsible for freeing the returned array.
 */
long* MessageService_get_contacts(int* count);

/**
 * @brief Gets the username for a given user ID.
 * This is a blocking call that sends the request and waits for the response.
 * @param userId The ID of the user to get info for.
 * @param out_username Buffer to store the username (should be at least 256 bytes).
 * @param buffer_size Size of the out_username buffer.
 * @return true on success, false on failure.
 */
bool MessageService_get_user_info(long userId, char* out_username, int buffer_size);

/**
 * @brief Searches for a user by their exact username.
 * This is a blocking call that sends the request and waits for the response.
 * @param username The username to search for.
 * @param out_userId Pointer to store the found user's ID.
 * @param out_username Buffer to store the found username (should be at least 256 bytes).
 * @param buffer_size Size of the out_username buffer.
 * @return true if user found, false otherwise.
 */
bool MessageService_search_user(const char* username, long* out_userId, char* out_username, int buffer_size);

#endif // MESSAGE_SERVICE_H
