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

int GroupService_join_group(long groupId, long userId) {
    // First check if group exists
    char group_name[256];
    if (GroupService_get_group_name(groupId, group_name, sizeof(group_name)) != 0) {
        fprintf(stderr, "GroupService Error: Group %ld not found.\n", groupId);
        return -1;
    }

    // Check if user is already a member
    PeachRecordSet* members = GroupService_get_group_members(groupId);
    if (members != NULL) {
        for (PeachRecord* rec = members->head; rec != NULL; rec = rec->next) {
            long member_userId = atol(rec->fields[2]); // userId is the 3rd field
            if (member_userId == userId) {
                Peach_free_record_set(members);
                fprintf(stderr, "GroupService: User %ld already in group %ld.\n", userId, groupId);
                return -1; // Already a member
            }
        }
        Peach_free_record_set(members);
    }

    // Add user to group
    long new_id = Peach_get_highest_key("groupusers") + 1;
    char record_str[256];
    snprintf(record_str, sizeof(record_str), "%ld^%ld^%ld", new_id, groupId, userId);

    if (Peach_write_record("groupusers", record_str) != 0) {
        fprintf(stderr, "GroupService Error: Failed to add user to group.\n");
        return -1;
    }

    printf("GroupService: User %ld joined group %ld.\n", userId, groupId);
    return 0;
}

long* GroupService_get_user_groups(long userId, int* count) {
    *count = 0;
    PeachRecordSet* all_records = Peach_read_all_records("groupusers");
    if (all_records == NULL) {
        return NULL;
    }

    long temp_groups[1024];
    int group_count = 0;

    for (PeachRecord* rec = all_records->head; rec != NULL; rec = rec->next) {
        long record_userId = atol(rec->fields[2]); // userId is the 3rd field
        if (record_userId == userId && group_count < 1024) {
            long groupId = atol(rec->fields[1]); // groupId is the 2nd field
            temp_groups[group_count++] = groupId;
        }
    }

    Peach_free_record_set(all_records);

    if (group_count == 0) {
        return NULL;
    }

    long* result = malloc(group_count * sizeof(long));
    if (result == NULL) {
        return NULL;
    }

    memcpy(result, temp_groups, group_count * sizeof(long));
    *count = group_count;
    return result;
}

int GroupService_get_group_name(long groupId, char* out_name, int buffer_size) {
    if (out_name == NULL || buffer_size <= 0) return -1;
    out_name[0] = '\0';

    PeachRecordSet* all_groups = Peach_read_all_records("groups");
    if (all_groups == NULL) {
        return -1;
    }

    int found = -1;
    for (PeachRecord* rec = all_groups->head; rec != NULL; rec = rec->next) {
        long record_groupId = atol(rec->fields[0]); // id is the 1st field
        if (record_groupId == groupId) {
            strncpy(out_name, rec->fields[1], buffer_size - 1); // name is the 2nd field
            out_name[buffer_size - 1] = '\0';
            found = 0;
            break;
        }
    }

    Peach_free_record_set(all_groups);
    return found;
}

PeachRecordSet* GroupService_get_group_history(long groupId) {
    PeachRecordSet* all_records = Peach_read_all_records("groupmessages");
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
        
        // fields: id^groupId^senderId^message^time
        long record_groupId = atol(current->fields[1]);

        if (record_groupId == groupId) {
            if (filtered_set->head == NULL) {
                filtered_set->head = current;
            } else {
                filtered_tail->next = current;
            }
            filtered_tail = current;
            current->next = NULL;
            filtered_set->record_count++;
        } else {
            free(current->fields[0]);
            free(current->fields);
            free(current);
        }
        current = next;
    }

    free(all_records);
    return filtered_set;
}
