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

/**
 * @brief Joins a user to an existing group.
 * 
 * @param groupId The ID of the group to join.
 * @param userId The ID of the user joining.
 * @return 0 on success, -1 on failure (group not found or already a member).
 */
int GroupService_join_group(long groupId, long userId);

/**
 * @brief Gets all groups that a user is a member of.
 * 
 * @param userId The ID of the user.
 * @param count Pointer to store the count of groups.
 * @return Array of group IDs, or NULL on failure. Caller must free.
 */
long* GroupService_get_user_groups(long userId, int* count);

/**
 * @brief Gets the group name by ID.
 * 
 * @param groupId The ID of the group.
 * @param out_name Buffer to store the group name.
 * @param buffer_size Size of the buffer.
 * @return 0 on success, -1 on failure.
 */
int GroupService_get_group_name(long groupId, char* out_name, int buffer_size);

/**
 * @brief Retrieves message history for a group.
 * 
 * @param groupId The ID of the group.
 * @return A PeachRecordSet containing messages, or NULL on failure.
 */
PeachRecordSet* GroupService_get_group_history(long groupId);

#endif // GROUP_SERVICE_H
