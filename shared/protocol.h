#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum { JOIN_ROOM, TEXT } MsgType;

typedef struct {
    MsgType type;
    char username[50];
    char room_code[10];
    char content[256];
} Packet;

#endif