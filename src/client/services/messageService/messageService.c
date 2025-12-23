#include "messageService.h"
#include "../networkService/networkService.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For atol

// Helper function to send a command and wait for a specific response prefix
static bool send_command_and_wait_for_response(const char* command, const char* success_prefix) {
    // 1. Clear any old response from the buffer
    Network_clear_response();

    // 2. Send the new command
    if (Network_send(command) != 0) {
        fprintf(stderr, "MessageService: Failed to send command to network service.\n");
        return false;
    }

    char response[1024];
    // 3. Poll for a response for a limited time (e.g., 20 * 100ms = 2 seconds)
    for (int i = 0; i < 20; i++) {
        usleep(100000); // Wait for 100ms

        memset(response, 0, sizeof(response));
        Network_get_response(response, sizeof(response));

        if (strlen(response) > 0) {
            // We got a response, check if it's the one we want
            if (strncmp(response, success_prefix, strlen(success_prefix)) == 0) {
                return true; // Success!
            } else {
                // It's a different response (e.g., an error), so we fail
                fprintf(stderr, "MessageService: Received unexpected response: %s\n", response);
                return false;
            }
        }
    }

    // 4. If we get here, we timed out waiting for a response
    fprintf(stderr, "MessageService: Timed out waiting for server response.\n");
    return false;
}

bool MessageService_send_dm(long receiverId, const char* message) {
    if (message == NULL || strlen(message) == 0) return false;

    char command[2048];
    snprintf(command, sizeof(command), "SEND_DM^%ld^%s", receiverId, message);

    return send_command_and_wait_for_response(command, "SEND_DM_SUCCESS");
}

char* MessageService_get_history(long contactId) {
    // 1. Format the command
    char command[1024];
    snprintf(command, sizeof(command), "GET_DM_HISTORY^%ld", contactId);

    // 2. Clear previous response and send command
    Network_clear_response();
    if (Network_send(command) != 0) {
        fprintf(stderr, "MessageService: Failed to send command for history.\n");
        return NULL;
    }

    // 3. Poll for a response
    // The history can be large, so we use a larger buffer.
    char response[16384]; // 16KB buffer for history
    const char* success_prefix = "HISTORY_DATA^";

    for (int i = 0; i < 40; i++) { // Wait up to 4 seconds for potentially large data
        usleep(100000); // Wait 100ms

        memset(response, 0, sizeof(response));
        Network_get_response(response, sizeof(response));

        if (strlen(response) > 0) {
            if (strncmp(response, success_prefix, strlen(success_prefix)) == 0) {
                // Success! Return the data part of the response.
                char* data = response + strlen(success_prefix);
                return strdup(data); // Return a heap-allocated copy
            } else {
                fprintf(stderr, "MessageService: Received unexpected history response: %s\n", response);
                return NULL;
            }
        }
    }
    
    fprintf(stderr, "MessageService: Timed out waiting for history response.\n");
    return NULL;
}

long* MessageService_get_contacts(int* count) {
    *count = 0;
    const char* command = "GET_CONTACTS";

    Network_clear_response();
    if (Network_send(command) != 0) {
        fprintf(stderr, "MessageService: Failed to send GET_CONTACTS command.\n");
        return NULL;
    }

    char response[8192]; // Buffer for contact list
    const char* success_prefix = "CONTACTS_DATA^";

    for (int i = 0; i < 20; i++) { // 2 second timeout
        usleep(100000);
        memset(response, 0, sizeof(response));
        Network_get_response(response, sizeof(response));

        if (strlen(response) > 0) {
            if (strncmp(response, success_prefix, strlen(success_prefix)) == 0) {
                char* data_str = response + strlen(success_prefix);
                if (strlen(data_str) == 0) {
                    return NULL; // No contacts
                }

                // First pass: count the contacts to allocate exact memory
                int num_contacts = 0;
                char* data_copy_for_count = strdup(data_str);
                char* token_count = strtok(data_copy_for_count, ",");
                while (token_count != NULL) {
                    num_contacts++;
                    token_count = strtok(NULL, ",");
                }
                free(data_copy_for_count);

                if (num_contacts == 0) {
                    return NULL;
                }

                // Second pass: allocate memory and parse IDs
                long* contacts_array = malloc(num_contacts * sizeof(long));
                if (contacts_array == NULL) {
                    return NULL; // Malloc failed
                }

                int index = 0;
                char* data_copy_for_parse = strdup(data_str);
                char* token_parse = strtok(data_copy_for_parse, ",");
                while (token_parse != NULL && index < num_contacts) {
                    contacts_array[index++] = atol(token_parse);
                    token_parse = strtok(NULL, ",");
                }
                free(data_copy_for_parse);

                *count = num_contacts;
                return contacts_array;

            } else {
                fprintf(stderr, "MessageService: Received unexpected contacts response: %s\n", response);
                return NULL;
            }
        }
    }

    fprintf(stderr, "MessageService: Timed out waiting for contacts response.\n");
    return NULL;
}

bool MessageService_get_user_info(long userId, char* out_username, int buffer_size) {
    if (out_username == NULL || buffer_size <= 0) return false;
    out_username[0] = '\0';

    char command[256];
    snprintf(command, sizeof(command), "GET_USER_INFO^%ld", userId);

    Network_clear_response();
    if (Network_send(command) != 0) {
        fprintf(stderr, "MessageService: Failed to send GET_USER_INFO command.\n");
        return false;
    }

    char response[1024];
    const char* success_prefix = "USER_INFO^";

    for (int i = 0; i < 20; i++) { // 2 second timeout
        usleep(100000);
        memset(response, 0, sizeof(response));
        Network_get_response(response, sizeof(response));

        if (strlen(response) > 0) {
            if (strncmp(response, success_prefix, strlen(success_prefix)) == 0) {
                // Parse: USER_INFO^userId^username
                char* data_str = response + strlen(success_prefix);
                char* saveptr = NULL;
                char* userId_str = strtok_r(data_str, "^", &saveptr);
                char* username_str = strtok_r(NULL, "^", &saveptr);

                if (username_str != NULL) {
                    strncpy(out_username, username_str, buffer_size - 1);
                    out_username[buffer_size - 1] = '\0';
                    return true;
                }
            }
            fprintf(stderr, "MessageService: Failed to get user info: %s\n", response);
            return false;
        }
    }

    fprintf(stderr, "MessageService: Timed out waiting for user info response.\n");
    return false;
}
