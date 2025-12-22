#include <sqlite3.h>
#include <stdio.h>
#include "../../shared/protocol.h"

sqlite3* init_db() {
    sqlite3 *db;
    sqlite3_open("chatroom.db", &db);
    
    // Bảng lưu lịch sử chat theo phòng
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS history("
                     "room_code TEXT, sender TEXT, message TEXT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);", 0, 0, 0);
    
    // Bảng lưu danh sách user đã từng vào phòng (để giữ tên)
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS room_users("
                     "room_code TEXT, username TEXT, UNIQUE(room_code, username));", 0, 0, 0);
    return db;
}

void save_message(sqlite3 *db, Packet p) {
    char query[512];
    sprintf(query, "INSERT INTO history (room_code, sender, message) VALUES('%s', '%s', '%s');", 
            p.room_code, p.username, p.content);
    sqlite3_exec(db, query, 0, 0, 0);
}

void register_user_to_room(sqlite3 *db, char* room, char* user) {
    char query[256];
    sprintf(query, "INSERT OR IGNORE INTO room_users (room_code, username) VALUES('%s', '%s');", room, user);
    sqlite3_exec(db, query, 0, 0, 0);
}