#include "authService.h"
#include "../networkService/networkService.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> // For usleep

// Global state for logged-in user
static long g_current_user_id = -1;

// Helper function to send a command and wait for a specific response prefix
static bool send_command_and_wait_for_response(const char* command, const char* success_prefix) {
    // 1. Clear any old response from the buffer
    Network_clear_response();

    // 2. Send the new command
    if (Network_send(command) != 0) {
        fprintf(stderr, "AuthService: Failed to send command to network service.\n");
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
                return false;
            }
        }
    }

    // 4. If we get here, we timed out waiting for a response
    fprintf(stderr, "AuthService: Timed out waiting for server response.\n");
    return false;
}


bool AuthService_register(const char* username, const char* password) {
    if (username == NULL || password == NULL) return false;

    char command[1024];
    snprintf(command, sizeof(command), "REGISTER^%s^%s", username, password);

    return send_command_and_wait_for_response(command, "REGISTER_SUCCESS");
}

bool AuthService_login(const char* username, const char* password, long* out_userId) {
    if (username == NULL || password == NULL) return false;
    if (out_userId != NULL) *out_userId = -1;

    char command[1024];
    snprintf(command, sizeof(command), "LOGIN^%s^%s", username, password);

    // 1. Clear any old response from the buffer
    Network_clear_response();

    // 2. Send the new command
    if (Network_send(command) != 0) {
        fprintf(stderr, "AuthService: Failed to send login command.\n");
        return false;
    }

    char response[1024];
    const char* success_prefix = "LOGIN_SUCCESS";
    
    // 3. Poll for a response
    for (int i = 0; i < 20; i++) {
        usleep(100000); // Wait for 100ms

        memset(response, 0, sizeof(response));
        Network_get_response(response, sizeof(response));

        if (strlen(response) > 0) {
            if (strncmp(response, success_prefix, strlen(success_prefix)) == 0) {
                // Parse user ID from response: LOGIN_SUCCESS^<userId>
                char* userId_str = strchr(response, '^');
                if (userId_str != NULL) {
                    userId_str++; // Skip the '^'
                    long userId = atol(userId_str);
                    g_current_user_id = userId;
                    if (out_userId != NULL) *out_userId = userId;
                    printf("AuthService: Login successful, user ID: %ld\n", userId);
                }
                return true;
            } else {
                fprintf(stderr, "AuthService: Login failed - %s\n", response);
                return false;
            }
        }
    }

    fprintf(stderr, "AuthService: Timed out waiting for login response.\n");
    return false;
}

long AuthService_get_current_user_id(void) {
    return g_current_user_id;
}
