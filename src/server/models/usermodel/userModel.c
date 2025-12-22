#include "userModel.h"
#include "../../services/peachdb/peachdb.h"
#include <stdio.h>
#include <string.h>

// Define the collection name used for users
static const char* USER_COLLECTION = "user";

// Helper to convert a PeachRecord to a User struct
static User* record_to_user(PeachRecord* record) {
    if (record == NULL || record->num_fields < 3) {
        return NULL;
    }
    User* user = malloc(sizeof(User));
    if (user == NULL) {
        return NULL;
    }
    user->id = atol(record->fields[0]);
    user->username = strdup(record->fields[1]);
    user->password = strdup(record->fields[2]);

    if (user->username == NULL || user->password == NULL) {
        User_free(user);
        return NULL;
    }
    return user;
}


long User_create(const User* user_data) {
    long next_id = Peach_get_highest_key(USER_COLLECTION) + 1;

    // Max record string size: long (20) + username (256) + password (256) + separators (3)
    char record_str[512]; 
    snprintf(record_str, sizeof(record_str), "%ld^%s^%s", 
             next_id, 
             user_data->username, 
             user_data->password);

    if (Peach_write_record(USER_COLLECTION, record_str) == 0) {
        return next_id; // Return the new ID on success
    }

    return -1; // Return -1 on failure
}

User* User_read(long id) {
    PeachRecordSet* record_set = Peach_read_all_records(USER_COLLECTION);
    if (record_set == NULL) {
        return NULL;
    }

    User* found_user = NULL;
    for (PeachRecord* current = record_set->head; current != NULL; current = current->next) {
        if (current->num_fields > 0) {
            long current_id = atol(current->fields[0]);
            if (current_id == id) {
                found_user = record_to_user(current);
                break; // Found the user, no need to continue
            }
        }
    }

    Peach_free_record_set(record_set); // Clean up the record set
    return found_user;
}

User* User_read_by_username(const char* username) {
    if (username == NULL) return NULL;

    PeachRecordSet* record_set = Peach_read_all_records(USER_COLLECTION);
    if (record_set == NULL) {
        return NULL;
    }

    User* found_user = NULL;
    for (PeachRecord* current = record_set->head; current != NULL; current = current->next) {
        if (current->num_fields > 1 && strcmp(current->fields[1], username) == 0) {
            found_user = record_to_user(current);
            break; // Found the user
        }
    }

    Peach_free_record_set(record_set);
    return found_user;
}

int User_update(const User* user_data) {
    if (user_data == NULL) return -1;

    char record_str[512];
    char key_str[21]; // Long max chars + null terminator

    snprintf(record_str, sizeof(record_str), "%ld^%s^%s", user_data->id, user_data->username, user_data->password);
    snprintf(key_str, sizeof(key_str), "%ld", user_data->id);

    return Peach_update_record(USER_COLLECTION, key_str, record_str);
}

int User_delete(long id) {
    char key_str[21];
    snprintf(key_str, sizeof(key_str), "%ld", id);
    return Peach_delete_record(USER_COLLECTION, key_str);
}

void User_free(User* user) {
    if (user != NULL) {
        free(user->username);
        free(user->password);
        free(user);
    }
}
