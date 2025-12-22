#ifndef MESSAGE_SERVICE_H
#define MESSAGE_SERVICE_H

#include <stdbool.h>
#include "../peachdb/peachdb.h" // For PeachRecordSet

/**
 * @brief Saves a new direct message to the database.
 * 
 * @param senderId The ID of the user sending the message.
 * @param receiverId The ID of the user receiving the message.
 * @param message The content of the message.
 * @return The ID of the newly created message on success, or -1 on failure.
 */
long MessageService_save_dm(long senderId, long receiverId, const char* message);

/**
 * @brief Retrieves the message history between two users.
 * 
 * @param userId1 The ID of the first user.
 * @param userId2 The ID of the second user.
 * @return A PeachRecordSet containing the messages, or NULL on failure.
 *         The caller is responsible for freeing the record set.
 */
PeachRecordSet* MessageService_get_history(long userId1, long userId2);

/**
 * @brief Gets a list of unique user IDs that the given user has conversed with.
 * 
 * @param userId The ID of the user whose contacts to find.
 * @param count A pointer to an integer where the number of contacts will be stored.
 * @return A dynamically allocated array of user IDs, or NULL on failure.
 *         The caller is responsible for freeing the returned array.
 */
long* MessageService_get_contacts(long userId, int* count);


#endif // MESSAGE_SERVICE_H
