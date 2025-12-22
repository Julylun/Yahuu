#include "sessionManager.h"
#include <string.h>
#include <stdio.h>

#define MAX_SESSIONS 100 // Maximum number of concurrent users

// Global array to store active sessions
static UserSession g_sessions[MAX_SESSIONS];
// Number of active sessions
static int g_session_count = 0;

void SessionManager_init() {
    memset(g_sessions, 0, sizeof(g_sessions));
    g_session_count = 0;
    printf("Session Manager initialized.\n");
}

int SessionManager_add(long userId, const char* username, int socket_fd) {
    if (g_session_count >= MAX_SESSIONS) {
        fprintf(stderr, "SessionManager Error: Maximum number of sessions reached.\n");
        return -1;
    }

    // Check if user or socket is already in a session
    for (int i = 0; i < g_session_count; i++) {
        if (g_sessions[i].userId == userId || g_sessions[i].socket_fd == socket_fd) {
            fprintf(stderr, "SessionManager Warning: User %ld (socket %d) is already in a session. Updating socket.\n", userId, socket_fd);
            // Update socket in case of re-login before disconnect
            g_sessions[i].socket_fd = socket_fd;
            strncpy(g_sessions[i].username, username, sizeof(g_sessions[i].username) - 1);
            return 0;
        }
    }


    g_sessions[g_session_count].userId = userId;
    g_sessions[g_session_count].socket_fd = socket_fd;
    strncpy(g_sessions[g_session_count].username, username, sizeof(g_sessions[g_session_count].username) - 1);
    g_sessions[g_session_count].username[sizeof(g_sessions[g_session_count].username) - 1] = '\0';
    
    g_session_count++;
    
    printf("Session added: UserID %ld, Username %s, Socket %d. Total sessions: %d\n", userId, username, socket_fd, g_session_count);
    return 0;
}

void SessionManager_remove_by_socket(int socket_fd) {
    int found_index = -1;
    for (int i = 0; i < g_session_count; i++) {
        if (g_sessions[i].socket_fd == socket_fd) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        printf("Session removed: UserID %ld, Username %s, Socket %d. \n", g_sessions[found_index].userId, g_sessions[found_index].username, socket_fd);
        // To remove, we swap the found element with the last element
        // and decrement the count. This is faster than shifting all elements.
        g_sessions[found_index] = g_sessions[g_session_count - 1];
        // Clear the now-unused last element
        memset(&g_sessions[g_session_count - 1], 0, sizeof(UserSession));
        g_session_count--;
        printf("Total sessions: %d\n", g_session_count);
    }
}

int SessionManager_get_socket(long userId) {
    for (int i = 0; i < g_session_count; i++) {
        if (g_sessions[i].userId == userId) {
            return g_sessions[i].socket_fd;
        }
    }
    return -1; // Not found
}

long SessionManager_get_user(int socket_fd) {
    for (int i = 0; i < g_session_count; i++) {
        if (g_sessions[i].socket_fd == socket_fd) {
            return g_sessions[i].userId;
        }
    }
    return -1; // Not found
}

const UserSession* SessionManager_get_session_by_socket(int socket_fd) {
    for (int i = 0; i < g_session_count; i++) {
        if (g_sessions[i].socket_fd == socket_fd) {
            return &g_sessions[i];
        }
    }
    return NULL; // Not found
}
