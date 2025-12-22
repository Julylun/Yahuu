#include <stdio.h>
#include <stdlib.h>
#include "src/server/services/peachdb/peachdb.h"

// Helper function to print the contents of a record set
void print_record_set(PeachRecordSet* record_set) {
    if (record_set == NULL) {
        printf("  Record set is NULL.\n");
        return;
    }

    printf("  Found %d records with %d fields each.\n", record_set->record_count, record_set->num_fields);
    
    PeachRecord* current = record_set->head;
    while (current != NULL) {
        printf("  - Record: ");
        for (int i = 0; i < current->num_fields; i++) {
            // Print field, add separator if not the last one
            printf("%s%s", current->fields[i], (i == current->num_fields - 1) ? "" : " | ");
        }
        printf("\n");
        current = current->next;
    }
}

int main() {
    printf("-----[ PeachDB Test Suite ]-----\n");

    printf("\n[0] Cleaning up previous test runs...\n");
    // This command removes the old database to ensure a clean test environment
    system("rm -rf peachdata");
    printf("  Done.\n\n");

    printf("[1] Initializing PeachDB service...\n");
    if (Peach_initPeachDb() != 0) {
        fprintf(stderr, "  FAILURE: PeachDB initialization failed.\n");
        return 1;
    }
    printf("  SUCCESS: PeachDB initialized.\n\n");

    printf("[2] Creating 'users' collection...\n");
    if (Peach_collection_create("users", "id^name^email") != 0) {
        fprintf(stderr, "  FAILURE: Could not create 'users' collection.\n");
        return 1;
    }
    printf("  SUCCESS: Collection 'users' created.\n\n");

    printf("[3] Writing records...\n");
    Peach_write_record("users", "1^Alice^alice@example.com");
    Peach_write_record("users", "2^Bob^bob@example.com");
    Peach_write_record("users", "3^Carol^carol@example.com");
    printf("  Finished writing initial records.\n\n");

    printf("[4] Testing Get Highest Key...\n");
    long highest_key = Peach_get_highest_key("users");
    if (highest_key != 3) {
        fprintf(stderr, "  FAILURE: Expected highest key to be 3, but got %ld.\n", highest_key);
    } else {
        printf("  SUCCESS: Correctly found highest key: %ld.\n", highest_key);
        long next_key = highest_key + 1;
        printf("  (Suggested next key for a new record would be %ld)\n", next_key);
    }
    printf("\n");

    printf("[5] Testing duplicate key constraint...\n");
    if (Peach_write_record("users", "2^David^david@example.com") == 0) {
        fprintf(stderr, "  FAILURE: Duplicate key '2' was written, but should have been rejected.\n");
    } else {
        printf("  SUCCESS: Duplicate key '2' was rejected as expected.\n\n");
    }

    printf("[6] Reading all records from 'users'...\n");
    PeachRecordSet* users = Peach_read_all_records("users");
    if (users == NULL) {
        fprintf(stderr, "  FAILURE: Failed to read records.\n");
    } else {
        printf("  SUCCESS: Read completed.\n");
        print_record_set(users);
    }
    printf("\n");

    printf("[7] Cleaning up memory...\n");
    Peach_free_record_set(users);
    printf("  SUCCESS: Memory freed.\n\n");

    printf("[8] Testing Update record...\n");
    if (Peach_update_record("users", "2", "2^Bob Smith^bob.s@work.com") != 0) {
        fprintf(stderr, "  FAILURE: Failed to update record '2'.\n");
    } else {
        printf("  SUCCESS: Update operation completed for key '2'.\n");
    }
    printf("  Reading records after update to verify:\n");
    users = Peach_read_all_records("users");
    print_record_set(users);
    Peach_free_record_set(users);
    printf("\n");

    printf("[9] Testing Delete record...\n");
    if (Peach_delete_record("users", "3") != 0) {
        fprintf(stderr, "  FAILURE: Failed to delete record '3'.\n");
    } else {
        printf("  SUCCESS: Delete operation completed for key '3'.\n");
    }
    printf("  Reading records after delete to verify:\n");
    users = Peach_read_all_records("users");
    print_record_set(users);
    Peach_free_record_set(users);
    printf("\n");

    printf("-----[ Test Finished ]-----\n");

    return 0;
}
