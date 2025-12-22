#ifndef GROUP_SERVICE_H
#define GROUP_SERVICE_H

#include <stdbool.h>

/**
 * @brief Sends a request to the server to create a new group.
 * This is a blocking call that sends the request and waits for a
 * confirmation from the server.
 * @param groupName The desired name for the new group.
 * @return The new group's ID on success, -1 on failure.
 */
long GroupService_create_group(const char* groupName);

/**
 * @brief Sends a message to a group.
 * This is a blocking call that sends the message and waits for a
 * confirmation from the server.
 * @param groupId The ID of the group to send the message to.
 * @param message The content of the message.
 * @return true on successful send and confirmation, false otherwise.
 */
bool GroupService_send_group_message(long groupId, const char* message);


#endif // GROUP_SERVICE_H

