#ifndef CHATSCREEN_H
#define CHATSCREEN_H

#include <raylib.h>
#include "../../../utils/constants/constants.h"
#include "../../../utils/fonts/fonts.h"
#include "../../../utils/utils/utils.h"
#include "../../components/components.h"

void ChatScreen_init();
void ChatScreen_load_data();
void drawChatScreen();
void drawChatList();
void drawChatInput();

#endif
