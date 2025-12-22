#include "groupService.h"
#include "../peachdb/peachdb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

long GroupService_create_group(const char* groupName, long ownerId) {
    if (groupName == NULL || strlen(groupName) == 0) {
        return -1;
    }

    // --- 1. Create the group entry in the 'groups' collection ---
    long new_groupId = Peach_get_highest_key("groups") + 1;
    
    char group_record_str[1024];
    snprintf(group_record_str, sizeof(group_record_str), "%ld^%s^%ld", new_groupId, groupName, ownerId);

    if (Peach_write_record("groups", group_record_str) != 0) {
        fprintf(stderr, "GroupService Error: Failed to write to 'groups' collection.\n");
        return -1;
    }

    // --- 2. Add the owner as the first member in the 'groupusers' collection ---
    long new_groupusers_id = Peach_get_highest_key("groupusers") + 1;

    char groupuser_record_str[1024];
    snprintf(groupuser_record_str, sizeof(groupuser_record_str), "%ld^%ld^%ld", new_groupusers_id, new_groupId, ownerId);

    if (Peach_write_record("groupusers", groupuser_record_str) != 0) {
        fprintf(stderr, "GroupService Warning: Created group '%s' (%ld), but failed to add owner as member.\n", groupName, new_groupId);
        // In a real database, we would roll back the previous insert.
        // Here, we accept the inconsistent state but still return the group ID.
    }

    printf("GroupService: Created group '%s' with ID %ld. Owner %ld added.\n", groupName, new_groupId, ownerId);
    return new_groupId;
}

long GroupService_save_group_message(long groupId, long senderId, const char* message) {
    if (message == NULL || strlen(message) == 0) {
        return -1;
    }

    // 1. Get the next available ID
    long next_id = Peach_get_highest_key("groupmessages") + 1;

    // 2. Get the current timestamp
    char time_str[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    // 3. Format the record string: id^groupId^senderId^message^time
    char record_str[2048];
    snprintf(record_str, sizeof(record_str), "%ld^%ld^%ld^%s^%s",
             next_id, groupId, senderId, message, time_str);

    // 4. Write the record to the database
    if (Peach_write_record("groupmessages", record_str) != 0) {
        fprintf(stderr, "GroupService Error: Failed to write group message to database.\n");
        return -1;
    }

    return next_id;
}

PeachRecordSet* GroupService_get_group_members(long groupId) {
    PeachRecordSet* all_records = Peach_read_all_records("groupusers");
    if (all_records == NULL) {
        return NULL;
    }

    PeachRecordSet* filtered_set = calloc(1, sizeof(PeachRecordSet));
    if (filtered_set == NULL) {
        Peach_free_record_set(all_records);
        return NULL;
    }

    filtered_set->num_fields = all_records->num_fields;
    PeachRecord* filtered_tail = NULL;

    PeachRecord* current = all_records->head;
    PeachRecord* next = NULL;

    while (current != NULL) {
        next = current->next;
        
        // The second field (index 1) is groupId
        long record_groupId = atol(current->fields[1]);

        if (record_groupId == groupId) {
            // Match found, move the node to the filtered list
            if (filtered_set->head == NULL) {
                filtered_set->head = current;
            } else {
                filtered_tail->next = current;
            }
            filtered_tail = current;
            current->next = NULL; // Unlink from any subsequent nodes
            filtered_set->record_count++;
        } else {
            // No match, free this record completely
            free(current->fields[0]); // The data string
            free(current->fields);    // The array of pointers
            free(current);            // The node struct
        }
        current = next;
    }

    // Free the original container struct (its nodes have been either moved or freed)
    free(all_records);

    return filtered_set;
}
