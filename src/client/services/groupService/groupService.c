#include "groupService.h"
#include "../networkService/networkService.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For atol

long GroupService_create_group(const char* groupName) {
    if (groupName == NULL || strlen(groupName) == 0) {
        return -1;
    }

    // 1. Format the command
    char command[1024];
    snprintf(command, sizeof(command), "CREATE_GROUP^%s", groupName);

    // 2. Clear previous response and send command
    Network_clear_response();
    if (Network_send(command) != 0) {
        fprintf(stderr, "GroupService: Failed to send command to network service.\n");
        return -1;
    }

    // 3. Poll for a response
    char response[1024];
    const char* success_prefix = "CREATE_GROUP_SUCCESS^";

    for (int i = 0; i < 20; i++) {
        usleep(100000); // Wait 100ms

        memset(response, 0, sizeof(response));
        Network_get_response(response, sizeof(response));

        if (strlen(response) > 0) {
            // Check for success response
            if (strncmp(response, success_prefix, strlen(success_prefix)) == 0) {
                // Success! Parse the group ID.
                char* id_str = response + strlen(success_prefix);
                return atol(id_str);
            } else {
                // It's a different response (e.g., an error), so we fail
                fprintf(stderr, "GroupService: Received failure response: %s\n", response);
                return -1;
            }
        }
    }

    // 4. If we get here, we timed out
    fprintf(stderr, "GroupService: Timed out waiting for server response.\n");
    return -1;
}

// Helper function to send a command and wait for a specific response prefix
static bool send_and_wait_for_prefix(const char* command, const char* success_prefix) {
    // 1. Clear any old response from the buffer
    Network_clear_response();

    // 2. Send the new command
    if (Network_send(command) != 0) {
        fprintf(stderr, "GroupService: Failed to send command to network service.\n");
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
                fprintf(stderr, "GroupService: Received unexpected response: %s\n", response);
                return false;
            }
        }
    }

    // 4. If we get here, we timed out waiting for a response
    fprintf(stderr, "GroupService: Timed out waiting for server response.\n");
    return false;
}

bool GroupService_send_group_message(long groupId, const char* message) {
    if (message == NULL || strlen(message) == 0) return false;

    char command[2048];
    snprintf(command, sizeof(command), "SEND_GROUP_MSG^%ld^%s", groupId, message);

    return send_and_wait_for_prefix(command, "SEND_GROUP_MSG_SUCCESS");
}
