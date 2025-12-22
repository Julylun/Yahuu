#ifndef GROUP_SERVICE_H
#define GROUP_SERVICE_H

#include "../peachdb/peachdb.h" // For PeachRecordSet

/**
 * @brief Creates a new group and adds the owner as the first member.
 * 
 * @param groupName The desired name for the new group.
 * @param ownerId The user ID of the group's creator.
 * @return The ID of the newly created group on success, or -1 on failure.
 */
long GroupService_create_group(const char* groupName, long ownerId);

/**
 * @brief Saves a new message sent to a group.
 * 
 * @param groupId The ID of the group receiving the message.
 * @param senderId The ID of the user sending the message.
 * @param message The content of the message.
 * @return The ID of the newly created message on success, or -1 on failure.
 */
long GroupService_save_group_message(long groupId, long senderId, const char* message);

/**
 * @brief Retrieves all members of a specific group.
 * 
 * @param groupId The ID of the group.
 * @return A PeachRecordSet containing user records from the 'groupusers' collection, or NULL on failure.
 *         The caller is responsible for freeing the record set.
 */
PeachRecordSet* GroupService_get_group_members(long groupId);


#endif // GROUP_SERVICE_H
