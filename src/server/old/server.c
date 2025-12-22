#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include "../../shared/protocol.h"

#define MAX_CLIENTS 100
#define MAX_ROOMS_PER_CLIENT 5

typedef struct {
    int socket;
    char username[50];
    char room_codes[MAX_ROOMS_PER_CLIENT][10];
    int room_count;
    int active;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
sqlite3 *db;

extern sqlite3* init_db();
extern void save_message(sqlite3 *db, Packet p);

void send_history(int sock, char* room_code) {
    sqlite3_stmt *res;
    char query[256];
    sprintf(query, "SELECT sender, message FROM history WHERE room_code='%s' ORDER BY timestamp ASC;", room_code);
    if (sqlite3_prepare_v2(db, query, -1, &res, 0) == SQLITE_OK) {
        while (sqlite3_step(res) == SQLITE_ROW) {
            Packet p_hist = {TEXT, "", "", ""};
            strcpy(p_hist.username, (char*)sqlite3_column_text(res, 0));
            strcpy(p_hist.room_code, room_code);
            strcpy(p_hist.content, (char*)sqlite3_column_text(res, 1));
            send(sock, &p_hist, sizeof(Packet), 0);
        }
    }
    sqlite3_finalize(res);
}

void *handle_client(void *arg) {
    int sock = *(int*)arg; free(arg);
    Packet p;
    int my_idx = -1;

    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].socket = sock;
            clients[i].active = 1;
            clients[i].room_count = 0;
            my_idx = i;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    while (recv(sock, &p, sizeof(Packet), 0) > 0) {
        if (p.type == JOIN_ROOM) {
            pthread_mutex_lock(&lock);
            strcpy(clients[my_idx].username, p.username);
            if (clients[my_idx].room_count < MAX_ROOMS_PER_CLIENT) {
                strcpy(clients[my_idx].room_codes[clients[my_idx].room_count++], p.room_code);
            }
            pthread_mutex_unlock(&lock);
            printf("Log: User [%s] joined Room [%s]\n", p.username, p.room_code);
            send_history(sock, p.room_code);
        } else if (p.type == TEXT) {
            save_message(db, p);
            pthread_mutex_lock(&lock);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active && clients[i].socket != sock) {
                    for (int r = 0; r < clients[i].room_count; r++) {
                        if (strcmp(clients[i].room_codes[r], p.room_code) == 0) {
                            send(clients[i].socket, &p, sizeof(Packet), 0);
                            break;
                        }
                    }
                }
            }
            pthread_mutex_unlock(&lock);
        }
    }
    pthread_mutex_lock(&lock);
    clients[my_idx].active = 0;
    pthread_mutex_unlock(&lock);
    close(sock);
    return NULL;
}

int main() {
    db = init_db();
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(8888), INADDR_ANY};
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 10);
    printf("Server started on port 8888...\n");
    while (1) {
        int *new_sock = malloc(sizeof(int));
        *new_sock = accept(server_fd, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, new_sock);
        pthread_detach(tid);
    }
    return 0;
}