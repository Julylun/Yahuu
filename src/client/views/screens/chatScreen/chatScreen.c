#include "chatScreen.h"
#include <raylib.h>


void drawChatListPanel()
{
    static bool isInitialized = false;
    static Texture2D Texture_ChatListIcon;

    if (!isInitialized) {
        Texture_ChatListIcon = loadResizeImage("assets/icon.png", 60, 60);
        isInitialized = true;
    }

    // ======== ICON ===========
    Rectangle Panel_ChatList = {
        0,
        0,
        WINDOW_SCREEN_WIDTH/4,
        WINDOW_SCREEN_HEIGHT
    };
    Rectangle Panel_ChatListHeader = {
        Panel_ChatList.x,
        Panel_ChatList.y,
        Panel_ChatList.width,
        180
    };
    DrawRectangleRec(Panel_ChatList, RED);
    DrawRectangleRec(Panel_ChatListHeader, GREEN);
    DrawTexture(Texture_ChatListIcon, 0, 0, WHITE);

    // ======== TITLE ===========
    Vector2 Measure_ChatListTitle = MeasureTextEx(Font_Opensans_Bold_20, WINDOW_TITLE, 20, 1);
    Vector2 Position_ChatListTitle = {
        Panel_ChatList.width/2 - Measure_ChatListTitle.x/2,
        30 - 12
    };
    DrawTextEx(
        Font_Opensans_Bold_20,WINDOW_TITLE,
        Position_ChatListTitle, 20, 1, WHITE
    );


    // ======== NEW CHAT BUTTON ===========
    Rectangle Position_NewChatButton = {
        Panel_ChatList.width/2 - 85,
        60,
        170,
        40
    };
    char Text_NewChatTitle[] = "New Chat";
    RoundedButton Button_NewChat = CreateRoundedButton(
        Position_NewChatButton,
        COLOR_DARKTHEME_PURPLE,
        COLOR_DARKTHEME_BLACK,
        COLOR_DARKTHEME_GRAY,
        Text_NewChatTitle,
        &Font_Opensans_Bold_17,
        15,
        0
    );
    DrawRoundedButton(Button_NewChat);


    // ========= Join Room ============
    Rectangle Position_JoinRoomButton = {
        Panel_ChatList.width/2 - 85,
        110,
        170,
        40
    };
    char Text_JoinRoomTitle[] = "Join Room";
    RoundedButton Button_JoinRoom = CreateRoundedButton(
        Position_JoinRoomButton,
        COLOR_DARKTHEME_PURPLE,
        COLOR_DARKTHEME_BLACK,
        COLOR_DARKTHEME_GRAY,
        Text_JoinRoomTitle,
        &Font_Opensans_Bold_17,
        15,
        0
    );
    DrawRoundedButton(Button_JoinRoom);
}




void drawChatSection()
{
    DrawRectangle(WINDOW_SCREEN_WIDTH/4, 0, WINDOW_SCREEN_WIDTH/4*3, WINDOW_SCREEN_HEIGHT, BLUE);
}
void drawChatScreen()
{
    drawChatListPanel();
    drawChatSection();
}
