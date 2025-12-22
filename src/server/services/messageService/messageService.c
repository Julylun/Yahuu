#include "messageService.h"
#include "../peachdb/peachdb.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

long MessageService_save_dm(long senderId, long receiverId, const char* message) {
    if (message == NULL || strlen(message) == 0) {
        return -1;
    }

    // 1. Get the next available ID
    long next_id = Peach_get_highest_key("messages") + 1;

    // 2. Get the current timestamp
    char time_str[20]; // Buffer for "YYYY-MM-DD HH:MM:SS"
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    // 3. Format the record string
    // Format: id^senderId^receiverId^message^time
    char record_str[2048];
    snprintf(record_str, sizeof(record_str), "%ld^%ld^%ld^%s^%s",
             next_id, senderId, receiverId, message, time_str);

    // 4. Write the record to the database
    if (Peach_write_record("messages", record_str) != 0) {
        fprintf(stderr, "MessageService Error: Failed to write direct message to database.\n");
        return -1;
    }

    return next_id; // Return the new message ID on success
}

PeachRecordSet* MessageService_get_history(long userId1, long userId2) {
    PeachRecordSet* all_records = Peach_read_all_records("messages");
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
        
        // fields: id^senderId^receiverId^message^time
        long senderId = atol(current->fields[1]);
        long receiverId = atol(current->fields[2]);

        bool is_match = (senderId == userId1 && receiverId == userId2) ||
                        (senderId == userId2 && receiverId == userId1);

        if (is_match) {
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

    // Free the original container struct
    free(all_records);

    return filtered_set;
}

long* MessageService_get_contacts(long userId, int* count) {
    *count = 0;
    PeachRecordSet* all_messages = Peach_read_all_records("messages");
    if (all_messages == NULL) {
        return NULL;
    }

    // Use a temporary, large-enough static array to find unique IDs.
    // A proper hash set would be better for performance with many contacts.
    long temp_contacts[4096];
    int contact_count = 0;
    
    for (PeachRecord* rec = all_messages->head; rec != NULL; rec = rec->next) {
        long senderId = atol(rec->fields[1]);
        long receiverId = atol(rec->fields[2]);
        long contactId = -1;

        if (senderId == userId) {
            contactId = receiverId;
        } else if (receiverId == userId) {
            contactId = senderId;
        }

        if (contactId != -1) {
            // Check if this contact is already in our list
            bool found = false;
            for (int i = 0; i < contact_count; i++) {
                if (temp_contacts[i] == contactId) {
                    found = true;
                    break;
                }
            }
            // If not found, add it
            if (!found && contact_count < 4096) {
                temp_contacts[contact_count++] = contactId;
            }
        }
    }

    Peach_free_record_set(all_messages);

    if (contact_count == 0) {
        return NULL;
    }

    // Create a heap-allocated array of the exact size
    long* final_contacts = malloc(contact_count * sizeof(long));
    if (final_contacts == NULL) {
        return NULL;
    }

    memcpy(final_contacts, temp_contacts, contact_count * sizeof(long));
    *count = contact_count;

    return final_contacts;
}
