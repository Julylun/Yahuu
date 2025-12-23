#include "socketService.h"
#include "../userService/userService.h" // Include the user service to handle logic
#include "../sessionManager/sessionManager.h"
#include "../messageService/messageService.h"
#include "../groupService/groupService.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h> // For inet_ntoa

// This is the new, command-aware client handler
static void* client_handler(void* socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);

    char client_message[2048];
    char response[2048];
    int read_size;

    // Define the separator for parsing commands
    const char* separator = "^";

    while ((read_size = recv(sock, client_message, sizeof(client_message) - 1, 0)) > 0) {
        client_message[read_size] = '\0';
        printf("Received from client %d: %s\n", sock, client_message);

        // Make a copy for strtok, as it modifies the string
        char* msg_copy = strdup(client_message);
        char* command = strtok(msg_copy, separator);

        if (command == NULL) {
            snprintf(response, sizeof(response), "ERROR^INVALID_COMMAND_FORMAT");
        } else if (strcmp(command, "REGISTER") == 0) {
            char* username = strtok(NULL, separator);
            char* password = strtok(NULL, separator);

            if (username && password) {
                long new_id = UserService_register(username, password);
                if (new_id > 0) {
                    snprintf(response, sizeof(response), "REGISTER_SUCCESS^%ld", new_id);
                } else {
                    snprintf(response, sizeof(response), "REGISTER_FAIL^USERNAME_TAKEN_OR_INVALID");
                }
            } else {
                snprintf(response, sizeof(response), "REGISTER_FAIL^INSUFFICIENT_ARGS");
            }
        } else if (strcmp(command, "LOGIN") == 0) {
            char* username = strtok(NULL, separator);
            char* password = strtok(NULL, separator);

            if (username && password) {
                User* user = UserService_login(username, password);
                if (user != NULL) {
                    SessionManager_add(user->id, user->username, sock);
                    snprintf(response, sizeof(response), "LOGIN_SUCCESS^%ld", user->id);
                    User_free(user); // Free the user struct after use
                } else {
                    snprintf(response, sizeof(response), "LOGIN_FAIL^INVALID_CREDENTIALS");
                }
            } else {
                snprintf(response, sizeof(response), "LOGIN_FAIL^INSUFFICIENT_ARGS");
            }
        } else if (strcmp(command, "SEND_DM") == 0) {
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                char* receiverId_str = strtok(NULL, separator);
                char* message = strtok(NULL, separator);

                if (receiverId_str && message) {
                    long receiverId = atol(receiverId_str);
                    long message_id = MessageService_save_dm(sender_session->userId, receiverId, message);

                    if (message_id > 0) {
                        snprintf(response, sizeof(response), "SEND_DM_SUCCESS^%ld", message_id);
                        
                        int receiver_socket = SessionManager_get_socket(receiverId);
                        if (receiver_socket != -1) {
                            char forward_msg[2048];
                            snprintf(forward_msg, sizeof(forward_msg), "RECEIVE_DM^%ld^%s", sender_session->userId, message);
                            printf("Forwarding DM from %ld to %ld (socket %d): %s\n", sender_session->userId, receiverId, receiver_socket, forward_msg);
                            write(receiver_socket, forward_msg, strlen(forward_msg));
                        }
                    } else {
                        snprintf(response, sizeof(response), "SEND_DM_FAIL^COULD_NOT_SAVE");
                    }
                } else {
                    snprintf(response, sizeof(response), "SEND_DM_FAIL^INSUFFICIENT_ARGS");
                }
            }
        } else if (strcmp(command, "CREATE_GROUP") == 0) {
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                char* groupName = strtok(NULL, separator);
                if (groupName) {
                    long new_groupId = GroupService_create_group(groupName, sender_session->userId);
                    if (new_groupId > 0) {
                        snprintf(response, sizeof(response), "CREATE_GROUP_SUCCESS^%ld^%s", new_groupId, groupName);
                    } else {
                        snprintf(response, sizeof(response), "CREATE_GROUP_FAIL");
                    }
                } else {
                    snprintf(response, sizeof(response), "CREATE_GROUP_FAIL^INSUFFICIENT_ARGS");
                }
            }
        } else if (strcmp(command, "JOIN_GROUP") == 0) {
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                char* groupId_str = strtok(NULL, separator);
                if (groupId_str) {
                    long groupId = atol(groupId_str);
                    char group_name[256];
                    if (GroupService_get_group_name(groupId, group_name, sizeof(group_name)) == 0) {
                        if (GroupService_join_group(groupId, sender_session->userId) == 0) {
                            snprintf(response, sizeof(response), "JOIN_GROUP_SUCCESS^%ld^%s", groupId, group_name);
                        } else {
                            snprintf(response, sizeof(response), "JOIN_GROUP_FAIL^ALREADY_MEMBER_OR_ERROR");
                        }
                    } else {
                        snprintf(response, sizeof(response), "JOIN_GROUP_FAIL^GROUP_NOT_FOUND");
                    }
                } else {
                    snprintf(response, sizeof(response), "JOIN_GROUP_FAIL^INSUFFICIENT_ARGS");
                }
            }
        } else if (strcmp(command, "GET_MY_GROUPS") == 0) {
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                int count = 0;
                long* groups = GroupService_get_user_groups(sender_session->userId, &count);
                
                if (groups != NULL && count > 0) {
                    size_t buffer_size = 8192;
                    char* groups_response = malloc(buffer_size);
                    if (groups_response) {
                        strcpy(groups_response, "MY_GROUPS_DATA^");
                        size_t current_len = strlen(groups_response);
                        
                        for (int i = 0; i < count; i++) {
                            char group_name[256];
                            if (GroupService_get_group_name(groups[i], group_name, sizeof(group_name)) == 0) {
                                int written = snprintf(groups_response + current_len, buffer_size - current_len,
                                                       "%ld,%s;", groups[i], group_name);
                                if (written > 0 && current_len + written < buffer_size) {
                                    current_len += written;
                                }
                            }
                        }
                        write(sock, groups_response, current_len);
                        free(groups_response);
                    }
                    free(groups);
                } else {
                    write(sock, "MY_GROUPS_DATA^", strlen("MY_GROUPS_DATA^"));
                }
                snprintf(response, sizeof(response), "");
            }
        } else if (strcmp(command, "GET_GROUP_HISTORY") == 0) {
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                char* groupId_str = strtok(NULL, separator);
                if (groupId_str) {
                    long groupId = atol(groupId_str);
                    PeachRecordSet* history = GroupService_get_group_history(groupId);
                    
                    if (history != NULL && history->record_count > 0) {
                        size_t buffer_size = 16384;
                        char* history_response = malloc(buffer_size);
                        if (history_response) {
                            strcpy(history_response, "GROUP_HISTORY_DATA^");
                            size_t current_len = strlen(history_response);
                            
                            for (PeachRecord* rec = history->head; rec != NULL; rec = rec->next) {
                                // fields: id^groupId^senderId^message^time
                                char* msg_senderId = rec->fields[2];
                                char* msg_content = rec->fields[3];
                                char* msg_time = rec->fields[4];
                                
                                int written = snprintf(history_response + current_len, buffer_size - current_len,
                                                       "%s,%s,%s;", msg_senderId, msg_content, msg_time);
                                if (written > 0 && current_len + written < buffer_size) {
                                    current_len += written;
                                } else {
                                    break;
                                }
                            }
                            write(sock, history_response, current_len);
                            free(history_response);
                        }
                        Peach_free_record_set(history);
                    } else {
                        write(sock, "GROUP_HISTORY_DATA^", strlen("GROUP_HISTORY_DATA^"));
                    }
                    snprintf(response, sizeof(response), "");
                } else {
                    snprintf(response, sizeof(response), "ERROR^GET_GROUP_HISTORY_FAIL^INSUFFICIENT_ARGS");
                }
            }
        } else if (strcmp(command, "SEND_GROUP_MSG") == 0) {
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                char* groupId_str = strtok(NULL, separator);
                char* message = strtok(NULL, separator);

                if (groupId_str && message) {
                    long groupId = atol(groupId_str);
                    long message_id = GroupService_save_group_message(groupId, sender_session->userId, message);

                    if (message_id > 0) {
                        snprintf(response, sizeof(response), "SEND_GROUP_MSG_SUCCESS^%ld", message_id);
                        
                        PeachRecordSet* members = GroupService_get_group_members(groupId);
                        if (members != NULL) {
                            for (PeachRecord* member_rec = members->head; member_rec != NULL; member_rec = member_rec->next) {
                                long member_userId = atol(member_rec->fields[2]); // userId is the 3rd field
                                
                                if (member_userId == sender_session->userId) continue; // Don't send to self

                                int member_socket = SessionManager_get_socket(member_userId);
                                if (member_socket != -1) {
                                    char forward_msg[2048];
                                    snprintf(forward_msg, sizeof(forward_msg), "RECEIVE_GROUP_MSG^%ld^%ld^%s", groupId, sender_session->userId, message);
                                    printf("Forwarding Group Msg to %ld (socket %d)\n", member_userId, member_socket);
                                    write(member_socket, forward_msg, strlen(forward_msg));
                                }
                            }
                            Peach_free_record_set(members);
                        }
                    } else {
                        snprintf(response, sizeof(response), "SEND_GROUP_MSG_FAIL^COULD_NOT_SAVE");
                    }
                } else {
                    snprintf(response, sizeof(response), "SEND_GROUP_MSG_FAIL^INSUFFICIENT_ARGS");
                }
            }
        } else if (strcmp(command, "GET_DM_HISTORY") == 0) {
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                char* contactId_str = strtok(NULL, separator);
                if (contactId_str) {
                    long contactId = atol(contactId_str);
                    PeachRecordSet* history = MessageService_get_history(sender_session->userId, contactId);
                    
                    // Manually send history response, then clear the standard response buffer
                    if (history != NULL && history->record_count > 0) {
                        size_t buffer_size = 16384; // 16KB buffer
                        char* history_response = malloc(buffer_size);
                        if (history_response) {
                            strcpy(history_response, "HISTORY_DATA^");
                            size_t current_len = strlen(history_response);

                            for (PeachRecord* rec = history->head; rec != NULL; rec = rec->next) {
                                // fields: id^senderId^receiverId^message^time
                                char* msg_senderId = rec->fields[1];
                                char* msg_content = rec->fields[3];
                                char* msg_time = rec->fields[4];
                                
                                int written = snprintf(history_response + current_len, buffer_size - current_len,
                                                       "%s,%s,%s;", msg_senderId, msg_content, msg_time);
                                
                                if (written > 0 && current_len + written < buffer_size) {
                                    current_len += written;
                                } else {
                                    break;
                                }
                            }
                            write(sock, history_response, current_len);
                            free(history_response);
                        }
                        Peach_free_record_set(history);
                    } else {
                        // No history or error, send empty data
                        write(sock, "HISTORY_DATA^", strlen("HISTORY_DATA^"));
                    }
                    snprintf(response, sizeof(response), ""); // Clear response buffer
                } else {
                    snprintf(response, sizeof(response), "ERROR^GET_DM_HISTORY_FAIL^INSUFFICIENT_ARGS");
                }
            }
        } else if (strcmp(command, "GET_CONTACTS") == 0) {
            snprintf(response, sizeof(response), ""); // Clear standard response
            const UserSession* sender_session = SessionManager_get_session_by_socket(sock);
            if (sender_session == NULL) {
                snprintf(response, sizeof(response), "ERROR^NOT_LOGGED_IN");
            } else {
                int count = 0;
                long* contacts = MessageService_get_contacts(sender_session->userId, &count);

                if (contacts != NULL && count > 0) {
                    size_t buffer_size = 8192; // 8KB buffer for contact list
                    char* contacts_response = malloc(buffer_size);
                    if (contacts_response) {
                        strcpy(contacts_response, "CONTACTS_DATA^");
                        size_t current_len = strlen(contacts_response);

                        for (int i = 0; i < count; i++) {
                            int written = snprintf(contacts_response + current_len, buffer_size - current_len,
                                                   "%ld,", contacts[i]);
                            
                            if (written > 0 && current_len + written < buffer_size) {
                                current_len += written;
                            } else {
                                break; // Buffer full or error
                            }
                        }
                        // Remove trailing comma if any
                        if (current_len > 0 && contacts_response[current_len - 1] == ',') {
                            contacts_response[current_len - 1] = '\0';
                            current_len--;
                        }

                        write(sock, contacts_response, current_len);
                        free(contacts_response);
                    }
                    free(contacts);
                } else {
                    // No contacts or error
                    write(sock, "CONTACTS_DATA^", strlen("CONTACTS_DATA^"));
                }
            }
        } else if (strcmp(command, "GET_USER_INFO") == 0) {
            char* userId_str = strtok(NULL, separator);
            if (userId_str) {
                long userId = atol(userId_str);
                User* user = User_read(userId);
                if (user != NULL) {
                    snprintf(response, sizeof(response), "USER_INFO^%ld^%s", user->id, user->username);
                    User_free(user);
                } else {
                    snprintf(response, sizeof(response), "USER_INFO_FAIL^USER_NOT_FOUND");
                }
            } else {
                snprintf(response, sizeof(response), "USER_INFO_FAIL^INSUFFICIENT_ARGS");
            }
        } else if (strcmp(command, "SEARCH_USER") == 0) {
            char* username = strtok(NULL, separator);
            if (username && strlen(username) > 0) {
                User* user = User_read_by_username(username);
                if (user != NULL) {
                    snprintf(response, sizeof(response), "SEARCH_USER_SUCCESS^%ld^%s", user->id, user->username);
                    User_free(user);
                } else {
                    snprintf(response, sizeof(response), "SEARCH_USER_FAIL^USER_NOT_FOUND");
                }
            } else {
                snprintf(response, sizeof(response), "SEARCH_USER_FAIL^INSUFFICIENT_ARGS");
            }
        } else {
            snprintf(response, sizeof(response), "ERROR^UNKNOWN_COMMAND");
        }
        
        free(msg_copy);

        // Send the response back to the client, if one was prepared
        if (strlen(response) > 0) {
            printf("Sending response to client %d: %s\n", sock, response);
            write(sock, response, strlen(response));
        }
        
        memset(client_message, 0, sizeof(client_message));
        memset(response, 0, sizeof(response));
    }

    if (read_size == 0) {
        printf("Client %d disconnected.\n", sock);
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    // Clean up the session before closing the socket
    SessionManager_remove_by_socket(sock);

    close(sock);
    return 0;
}


int Socket_init(int port, int max_connections) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // 1. Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    // 2. Set socket options to allow reusing the address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_fd);
        return -1;
    }

    // Initialize sockaddr_in structure
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 3. Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return -1;
    }

    // 4. Put the server socket in a passive mode to wait for clients
    if (listen(server_fd, max_connections) < 0) {
        perror("listen failed");
        close(server_fd);
        return -1;
    }

    printf("Server socket initialized. Listening on port %d\n", port);
    return server_fd;
}

void Socket_run(int server_fd) {
    printf("Server is running. Waiting for incoming connections...\n");

    struct sockaddr_in client_addr;
    int c = sizeof(struct sockaddr_in);
    int client_sock;

    while ((client_sock = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&c))) {
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }
        
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_t client_thread;
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&client_thread, NULL, client_handler, (void*)new_sock) < 0) {
            perror("could not create thread");
            free(new_sock);
            close(client_sock);
        } else {
            pthread_detach(client_thread);
            printf("Handler thread assigned for client %d.\n", client_sock);
        }
    }
    
    // If the loop exits, it's a critical failure.
    if (client_sock < 0) {
        perror("accept failed, shutting down server");
        close(server_fd);
    }
}
