/*==================[ PEACH DATABASE SPECIFICATION (v2) ]=========
 * Design decision: To optimize for write performance, record counts are not
 * stored in the database files. Instead, records are appended directly,
 * and the total count is determined at read-time (e.g., by counting lines).
 *
 * File structure:
 *   - peachdata/
 *     - collections/
 *         - {collection_name}.lpdb
 *     - index.mpdb
 *
 * index.mpdb format:
 *   Line 1: <number_of_collections>
 *   Line 2...N: <collection_name> <num_fields> <field1> ... <fieldN>
 *
 * {collection_name}.lpdb format:
 *   Line 1: <field1>^<field2>^...^<fieldN>
 *   Line 2...M: <value1>^<value2>^...^<valueN>
 ==========================================================*/

#include "peachdb.h"
#include <stdio.h>
#include <sys/stat.h> // For mkdir
#include <unistd.h>   // For access()
#include <errno.h>    // For errno
#include <string.h>   // For strerror
#include <stdlib.h>   // For malloc, free

// Define constants for paths
#define DB_ROOT_PATH "peachdata"
#define COLLECTIONS_PATH "peachdata/collections"
#define INDEX_PATH "peachdata/index.mpdb"

// Helper function to check if a directory exists and create it if not.
// Returns 0 on success, -1 on failure.
static int ensure_dir_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) != 0) {
            fprintf(stderr, "Error creating directory %s: %s\n", path, strerror(errno));
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Initialize the PeachDB service.
 * Checks for 'peachdata/' directory, 'peachdata/collections/' directory,
 * and 'peachdata/index.mpdb' file. Creates them if they don't exist.
 * @return 0 on success, -1 on failure.
 */
int Peach_initPeachDb() {
    // 1. Ensure 'peachdata' directory exists
    if (ensure_dir_exists(DB_ROOT_PATH) != 0) {
        return -1;
    }

    // 2. Ensure 'peachdata/collections' directory exists
    if (ensure_dir_exists(COLLECTIONS_PATH) != 0) {
        return -1;
    }

    // 3. Ensure 'index.mpdb' file exists
    if (access(INDEX_PATH, F_OK) == -1) {
        // File does not exist, create it with initial content "0"
        FILE* index_file = fopen(INDEX_PATH, "w");
        if (index_file == NULL) {
            fprintf(stderr, "Error creating file %s: %s\n", INDEX_PATH, strerror(errno));
            return -1;
        }
        fprintf(index_file, "0\n"); // Initial number of collections
        fclose(index_file);
    }

    return 0; // Success
}

// Helper function to count fields and replace '^' with space.
// This function modifies the input string `fields_str`.
static int count_and_prepare_fields(char* fields_str) {
    if (fields_str == NULL || *fields_str == '\0') {
        return 0;
    }
    int count = 1;
    char* p = fields_str;
    while (*p != '\0') {
        if (*p == '^') {
            count++;
            *p = ' '; // Replace '^' with a space for storing in index.mpdb
        }
        p++;
    }
    return count;
}

/**
 * @brief Creates a new collection with specified fields.
 * @param collection_name The name for the new collection.
 * @param fields A string containing field names separated by '^' (e.g., "id^name^age").
 * @return 0 on success, -1 on failure (e.g., collection already exists).
 */
int Peach_collection_create(const char* collection_name, const char* fields) {
    // --- 1. Read index file and check for existing collection ---
    FILE* index_file_read = fopen(INDEX_PATH, "r");
    if (index_file_read == NULL) {
        fprintf(stderr, "Error: Could not open index file %s for reading.\n", INDEX_PATH);
        return -1;
    }

    int num_collections;
    if (fscanf(index_file_read, "%d\n", &num_collections) != 1) {
        fprintf(stderr, "Error: Could not read number of collections from index.\n");
        fclose(index_file_read);
        return -1;
    }

    // Store existing lines in memory to write them back later
    char** existing_lines = malloc(sizeof(char*) * num_collections);
    if(num_collections > 0 && existing_lines == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for existing lines.\n");
        fclose(index_file_read);
        return -1;
    }

    char buffer[512];
    for (int i = 0; i < num_collections; i++) {
        if (fgets(buffer, sizeof(buffer), index_file_read) == NULL) {
            fprintf(stderr, "Warning: Index file may be corrupted. Unexpected EOF.\n");
            num_collections = i;
            break;
        }

        char temp_buffer[512];
        strcpy(temp_buffer, buffer);
        char* current_collection_name = strtok(temp_buffer, " ");

        if (current_collection_name != NULL && strcmp(current_collection_name, collection_name) == 0) {
            fprintf(stderr, "Error: Collection '%s' already exists.\n", collection_name);
            for (int j = 0; j < i; j++) {
                free(existing_lines[j]);
            }
            free(existing_lines);
            fclose(index_file_read);
            return -1;
        }
        existing_lines[i] = strdup(buffer);
    }
    fclose(index_file_read);

    // --- 2. Create the new collection file (.lpdb) ---
    char collection_path[256];
    snprintf(collection_path, sizeof(collection_path), "%s/%s.lpdb", COLLECTIONS_PATH, collection_name);

    FILE* collection_file = fopen(collection_path, "w");
    if (collection_file == NULL) {
        fprintf(stderr, "Error: Failed to create collection file %s.\n", collection_path);
        for (int i = 0; i < num_collections; i++) {
            free(existing_lines[i]);
        }
        free(existing_lines);
        return -1;
    }
    fprintf(collection_file, "%s\n", fields);
    fclose(collection_file);

    // --- 3. Write the new, updated index file ---
    FILE* index_file_write = fopen(INDEX_PATH, "w");
    if (index_file_write == NULL) {
        fprintf(stderr, "Error: Could not open index file %s for writing.\n", INDEX_PATH);
        remove(collection_path);
        for (int i = 0; i < num_collections; i++) {
            free(existing_lines[i]);
        }
        free(existing_lines);
        return -1;
    }

    fprintf(index_file_write, "%d\n", num_collections + 1);

    for (int i = 0; i < num_collections; i++) {
        fprintf(index_file_write, "%s", existing_lines[i]);
        free(existing_lines[i]);
    }
    free(existing_lines);

    char* fields_copy = strdup(fields);
    int num_fields = count_and_prepare_fields(fields_copy);
    fprintf(index_file_write, "%s %d %s\n", collection_name, num_fields, fields_copy);
    free(fields_copy);

    fclose(index_file_write);

    return 0; // Success
}

// Helper to get the key (the first field) from a record string.
// The caller must free the returned string.
static char* get_key_from_record(const char* record_str) {
    if (record_str == NULL) return NULL;
    const char* separator_pos = strchr(record_str, '^');
    size_t key_len;

    if (separator_pos == NULL) {
        // The whole string is the key if no separator is found.
        key_len = strlen(record_str);
    } else {
        key_len = separator_pos - record_str;
    }

    // Handle case where line might be empty or just a newline
    if (key_len == 0) {
        return NULL;
    }

    char* key = malloc(key_len + 1);
    if (key == NULL) return NULL;

    strncpy(key, record_str, key_len);
    key[key_len] = '\0';
    return key;
}

/**
 * @brief Appends a new record to a collection, ensuring the first field is a unique key.
 * @param collection_name The name of the collection.
 * @param record_str A string containing the record's data, with fields
 *                   separated by '^' (e.g., "1^John Doe^30").
 * @return 0 on success, -1 on failure (duplicate key or other error).
 */
int Peach_write_record(const char* collection_name, const char* record_str) {
    char collection_path[256];
    snprintf(collection_path, sizeof(collection_path), "%s/%s.lpdb", COLLECTIONS_PATH, collection_name);

    // Extract key from the new record
    char* new_key = get_key_from_record(record_str);
    if (new_key == NULL) {
        fprintf(stderr, "Error: Could not extract key from new record, or record is empty.\n");
        return -1;
    }

    FILE* collection_file = fopen(collection_path, "r");
    if (collection_file == NULL) {
        fprintf(stderr, "Error: Could not open collection '%s' for reading. It may not exist.\n", collection_name);
        free(new_key);
        return -1;
    }

    char buffer[1024];
    int is_duplicate = 0;

    // Skip header line
    fgets(buffer, sizeof(buffer), collection_file);

    // Check subsequent lines for duplicate keys
    while (fgets(buffer, sizeof(buffer), collection_file) != NULL) {
        // Remove newline character if it exists
        buffer[strcspn(buffer, "\n")] = 0;

        char* existing_key = get_key_from_record(buffer);
        if (existing_key != NULL) {
            if (strcmp(new_key, existing_key) == 0) {
                is_duplicate = 1;
                free(existing_key);
                break; // Found a duplicate
            }
            free(existing_key);
        }
    }
    fclose(collection_file);

    if (is_duplicate) {
        fprintf(stderr, "Error: Duplicate key '%s' found in collection '%s'.\n", new_key, collection_name);
        free(new_key);
        return -1;
    }

    // --- No duplicate found, proceed to append the record ---
    collection_file = fopen(collection_path, "a");
    if (collection_file == NULL) {
        fprintf(stderr, "Error: Could not re-open collection '%s' for appending.\n", collection_name);
        free(new_key);
        return -1;
    }

    if (fprintf(collection_file, "%s\n", record_str) < 0) {
        fprintf(stderr, "Error: Failed to write record to collection '%s'.\n", collection_name);
        fclose(collection_file);
        free(new_key);
        return -1;
    }

    fclose(collection_file);
    free(new_key);
    return 0; // Success
}

void Peach_free_record_set(PeachRecordSet* record_set) {
    if (record_set == NULL) return;

    PeachRecord* current = record_set->head;
    while (current != NULL) {
        PeachRecord* next = current->next;
        // The first field points to the start of the duplicated string for the whole record.
        // Freeing it frees the memory for all field data within that record.
        if (current->fields != NULL) {
            free(current->fields[0]);
            // Free the array of pointers itself.
            free(current->fields);
        }
        // Free the record struct itself.
        free(current);
        current = next;
    }
    // Finally, free the main container.
    free(record_set);
}

PeachRecordSet* Peach_read_all_records(const char* collection_name) {
    char collection_path[256];
    snprintf(collection_path, sizeof(collection_path), "%s/%s.lpdb", COLLECTIONS_PATH, collection_name);

    FILE* file = fopen(collection_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open collection file '%s' for reading.\n", collection_name);
        return NULL;
    }

    PeachRecordSet* record_set = calloc(1, sizeof(PeachRecordSet));
    if (record_set == NULL) {
        fclose(file);
        return NULL;
    }

    char buffer[1024];
    // Read header line to count fields
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return record_set; // Return empty set for an empty file
    }
    buffer[strcspn(buffer, "\n")] = 0;

    int num_fields = 0;
    if (strlen(buffer) > 0) {
        num_fields = 1;
        for (char* p = buffer; *p != '\0'; p++) {
            if (*p == '^') num_fields++;
        }
    }
    record_set->num_fields = num_fields;

    PeachRecord* tail = NULL; // To append new records efficiently

    // Read data lines
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) == 0) continue; // Skip empty lines

        PeachRecord* new_record = calloc(1, sizeof(PeachRecord));
        char* line_copy = strdup(buffer);
        char** fields_array = calloc(num_fields, sizeof(char*));

        if (new_record == NULL || line_copy == NULL || fields_array == NULL) {
            free(new_record);
            free(line_copy);
            free(fields_array);
            Peach_free_record_set(record_set);
            fclose(file);
            return NULL; // Memory allocation error
        }

        new_record->num_fields = num_fields;
        new_record->fields = fields_array;
        
        // Tokenize the line_copy by replacing '^' with '\0'
        fields_array[0] = line_copy;
        int field_index = 1;
        for (char* p = line_copy; *p != '\0' && field_index < num_fields; p++) {
            if (*p == '^') {
                *p = '\0';
                fields_array[field_index++] = p + 1;
            }
        }

        // Append to linked list
        if (record_set->head == NULL) {
            record_set->head = new_record;
        } else {
            tail->next = new_record;
        }
        tail = new_record;
        record_set->record_count++;
    }

    fclose(file);
    return record_set;
}

int Peach_delete_record(const char* collection_name, const char* key) {
    char original_path[256];
    char temp_path[256 + 4]; // for ".tmp"

    snprintf(original_path, sizeof(original_path), "%s/%s.lpdb", COLLECTIONS_PATH, collection_name);
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", original_path);

    FILE* original_file = fopen(original_path, "r");
    if (original_file == NULL) {
        fprintf(stderr, "Error: Cannot open collection '%s' to delete record.\n", collection_name);
        return -1;
    }

    FILE* temp_file = fopen(temp_path, "w");
    if (temp_file == NULL) {
        fprintf(stderr, "Error: Cannot create temporary file for deletion.\n");
        fclose(original_file);
        return -1;
    }

    char buffer[1024];
    int record_found = 0;

    // Copy header
    if (fgets(buffer, sizeof(buffer), original_file) != NULL) {
        fputs(buffer, temp_file);
    }

    // Copy records, skipping the one to delete
    while (fgets(buffer, sizeof(buffer), original_file) != NULL) {
        char clean_buffer[1024];
        strcpy(clean_buffer, buffer);
        clean_buffer[strcspn(clean_buffer, "\n")] = 0;

        char* record_key = get_key_from_record(clean_buffer);
        if (record_key != NULL) {
            if (strcmp(key, record_key) == 0) {
                record_found = 1; // Found it, so we skip writing this line
            } else {
                fputs(buffer, temp_file); // Not the key, so write it to temp file
            }
            free(record_key);
        } else {
             fputs(buffer, temp_file); // Write lines that don't have a parseable key (e.g., empty lines)
        }
    }

    fclose(original_file);
    fclose(temp_file);

    if (!record_found) {
        remove(temp_path); // Delete the useless temp file
        return -1; // Return -1 to indicate "not found"
    }

    // Replace the original file with the temp file
    if (remove(original_path) != 0) {
        fprintf(stderr, "Error: Could not delete original file '%s'.\n", original_path);
        remove(temp_path);
        return -1;
    }
    if (rename(temp_path, original_path) != 0) {
        fprintf(stderr, "Error: Could not rename temp file to '%s'.\n", original_path);
        return -1;
    }

    return 0; // Success
}

int Peach_update_record(const char* collection_name, const char* key, const char* new_record_str) {
    // Safety check: key in new record must match the key parameter
    char* new_key = get_key_from_record(new_record_str);
    if (new_key == NULL || strcmp(key, new_key) != 0) {
        fprintf(stderr, "Error: Key in new record data ('%s') does not match the target key ('%s').\n", new_key ? new_key : "NULL", key);
        free(new_key);
        return -1;
    }
    free(new_key);

    char original_path[256];
    char temp_path[256 + 4];

    snprintf(original_path, sizeof(original_path), "%s/%s.lpdb", COLLECTIONS_PATH, collection_name);
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", original_path);

    FILE* original_file = fopen(original_path, "r");
    if (original_file == NULL) {
        fprintf(stderr, "Error: Cannot open collection '%s' to update record.\n", collection_name);
        return -1;
    }

    FILE* temp_file = fopen(temp_path, "w");
    if (temp_file == NULL) {
        fprintf(stderr, "Error: Cannot create temporary file for update.\n");
        fclose(original_file);
        return -1;
    }

    char buffer[1024];
    int record_found = 0;

    // Copy header
    if (fgets(buffer, sizeof(buffer), original_file) != NULL) {
        fputs(buffer, temp_file);
    }

    // Read records, update the target record, and copy the rest
    while (fgets(buffer, sizeof(buffer), original_file) != NULL) {
        char clean_buffer[1024];
        strcpy(clean_buffer, buffer);
        clean_buffer[strcspn(clean_buffer, "\n")] = 0;

        char* record_key = get_key_from_record(clean_buffer);
        if (record_key != NULL) {
            if (strcmp(key, record_key) == 0) {
                record_found = 1;
                // Write the new record string instead of the old one
                fprintf(temp_file, "%s\n", new_record_str);
            } else {
                // Not the key, so write the original line
                fputs(buffer, temp_file);
            }
            free(record_key);
        } else {
             fputs(buffer, temp_file);
        }
    }

    fclose(original_file);
    fclose(temp_file);

    if (!record_found) {
        remove(temp_path);
        return -1; // Record to update was not found
    }

    if (remove(original_path) != 0) {
        fprintf(stderr, "Error: Could not delete original file '%s' for update.\n", original_path);
        remove(temp_path);
        return -1;
    }
    if (rename(temp_path, original_path) != 0) {
        fprintf(stderr, "Error: Could not rename temp file to '%s' for update.\n", original_path);
        return -1;
    }

    return 0; // Success
}

long Peach_get_highest_key(const char* collection_name) {
    char collection_path[256];
    snprintf(collection_path, sizeof(collection_path), "%s/%s.lpdb", COLLECTIONS_PATH, collection_name);

    FILE* file = fopen(collection_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open collection '%s' to get highest key.\n", collection_name);
        return -1; // Indicate error
    }

    char buffer[1024];
    long highest_key = 0;

    // Skip header line
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return 0; // Collection is empty or just has a header
    }

    // Read data lines
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char* key_str = get_key_from_record(buffer);
        if (key_str != NULL) {
            long current_key = atol(key_str); // Convert string to long
            if (current_key > highest_key) {
                highest_key = current_key;
            }
            free(key_str);
        }
    }

    fclose(file);
    return highest_key;
}