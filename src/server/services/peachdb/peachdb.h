#ifndef PEACHDB_H
#define PEACHDB_H

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

// Represents a single record (row) in a collection.
typedef struct PeachRecord {
    char** fields;          // An array of strings for each field's value.
    int num_fields;         // The number of fields in this record.
    struct PeachRecord* next; // Pointer to the next record in a linked list.
} PeachRecord;

// Represents a set of records returned from a query.
typedef struct {
    PeachRecord* head;      // The first record in the linked list.
    int record_count;       // The total number of records in the set.
    int num_fields;         // The number of fields per record.
} PeachRecordSet;


/**
 * @brief Initialize the PeachDB service.
 * Checks for 'peachdata/' directory, 'peachdata/collections/' directory,
 * and 'peachdata/index.mpdb' file. Creates them if they don't exist.
 * @return 0 on success, -1 on failure.
 */
int Peach_initPeachDb();

/**
 * @brief Creates a new collection with specified fields.
 * @param collection_name The name for the new collection.
 * @param fields A string containing field names separated by '^' (e.g., "id^name^age").
 * @return 0 on success, -1 on failure (e.g., collection already exists).
 */
int Peach_collection_create(const char* collection_name, const char* fields);

/**
 * @brief Deletes a collection.
 * @param collection_name The name of the collection to delete.
 * @return 0 on success, -1 on failure.
 */
int Peach_collection_delete(const char* collection_name);

/**
 * @brief Appends a new record to the end of a collection.
 * @param collection_name The name of the collection.
 * @param record_str A string containing the record's data, with fields
 *                   separated by '^' (e.g., "1^John Doe^30").
 * @return 0 on success, -1 on failure (e.g., collection not found).
 */
int Peach_write_record(const char* collection_name, const char* record_str);

/**
 * @brief Reads all records from a collection.
 * @param collection_name The name of the collection to read from.
 * @return A pointer to a PeachRecordSet containing all records, or NULL on failure.
 *         The caller is responsible for freeing the returned structure using
 *         Peach_free_record_set().
 */
PeachRecordSet* Peach_read_all_records(const char* collection_name);

/**
 * @brief Frees the memory allocated for a PeachRecordSet, including all its records and fields.
 * @param record_set The record set to free.
 */
void Peach_free_record_set(PeachRecordSet* record_set);

/**
 * @brief Deletes a record from a collection identified by its key.
 * @param collection_name The name of the collection.
 * @param key The key of the record to delete.
 * @return 0 on success, -1 if the record is not found or an error occurs.
 */
int Peach_delete_record(const char* collection_name, const char* key);

/**
 * @brief Updates a record in a collection identified by its key.
 * The key in the new record string must match the key parameter to prevent
 * accidental key changes.
 * @param collection_name The name of the collection.
 * @param key The key of the record to update.
 * @param new_record_str The full string for the new record data.
 * @return 0 on success, -1 if record not found or an error occurs.
 */
int Peach_update_record(const char* collection_name, const char* key, const char* new_record_str);

/**
 * @brief Finds the highest numerical key in a collection.
 * Assumes the first field is a key that can be converted to a long integer.
 * @param collection_name The name of the collection.
 * @return The highest key value found. Returns 0 if the collection is empty
 *         or if no numerical keys are found. Returns -1 on error (e.g., file not found).
 */
long Peach_get_highest_key(const char* collection_name);

#endif // PEACHDB_H