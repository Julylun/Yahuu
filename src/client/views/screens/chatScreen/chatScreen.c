#include "chatScreen.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../../services/networkService/networkService.h"
#include "../../../services/messageService/messageService.h"
#include "../../../services/authService/authService.h"
#include "../../components/components.h"

// --- Module State ---
typedef struct {
    long sender_id;
    char message[1024];
    char time[32];
    bool is_me; // NOTE: is_me logic requires knowing our own user ID.
} ChatMessage;

static long g_my_user_id = -1; // Placeholder for the current user's ID
static long g_contact_ids[1024];
static int g_contact_count = 0;

static long g_current_chat_contact_id = -1;
static ChatMessage g_chat_messages[2048];
static int g_chat_message_count = 0;
static bool g_should_scroll_to_bottom = false; // Flag to auto-scroll when new messages arrive

// --- Helper Functions ---
static void parse_and_load_messages(const char* history_data) {
    g_chat_message_count = 0;
    if (history_data == NULL || strlen(history_data) == 0) {
        return;
    }

    char* history_copy = strdup(history_data);
    char* outer_saveptr = NULL;
    char* inner_saveptr = NULL;
    
    char* msg_token = strtok_r(history_copy, ";", &outer_saveptr);

    while (msg_token != NULL && g_chat_message_count < 2048) {
        // Make a copy of msg_token for inner parsing
        char* msg_copy = strdup(msg_token);
        
        char* sender_str = strtok_r(msg_copy, ",", &inner_saveptr);
        char* message_str = strtok_r(NULL, ",", &inner_saveptr);
        char* time_str = strtok_r(NULL, ",", &inner_saveptr);

        if (sender_str && message_str && time_str) {
            ChatMessage* msg = &g_chat_messages[g_chat_message_count];
            msg->sender_id = atol(sender_str);
            strncpy(msg->message, message_str, sizeof(msg->message) - 1);
            msg->message[sizeof(msg->message) - 1] = '\0';
            strncpy(msg->time, time_str, sizeof(msg->time) - 1);
            msg->time[sizeof(msg->time) - 1] = '\0';
            msg->is_me = (msg->sender_id == g_my_user_id);
            g_chat_message_count++;
        }
        
        free(msg_copy);
        msg_token = strtok_r(NULL, ";", &outer_saveptr);
    }
    free(history_copy);
    g_should_scroll_to_bottom = true; // Auto-scroll when loading chat history
    printf("ChatScreen: Parsed and loaded %d messages.\n", g_chat_message_count);
}


static void add_contact_if_not_exists(long contactId) {
    bool found = false;
    for (int i = 0; i < g_contact_count; i++) {
        if (g_contact_ids[i] == contactId) {
            found = true;
            break;
        }
    }
    if (!found && g_contact_count < 1024) {
        g_contact_ids[g_contact_count++] = contactId;
    }
}

// --- Async Message Handler ---
static void handle_async_messages(const char* message) {
    char* msg_copy = strdup(message);
    if (msg_copy == NULL) return;

    const char* separator = "^";
    char* command = strtok(msg_copy, separator);

    if (command != NULL) {
        if (strcmp(command, "RECEIVE_DM") == 0) {
            char* senderId_str = strtok(NULL, separator);
            char* msg_content = strtok(NULL, separator);
            if (senderId_str && msg_content) {
                long senderId = atol(senderId_str);
                add_contact_if_not_exists(senderId);

                if (senderId == g_current_chat_contact_id && g_chat_message_count < 2048) {
                    ChatMessage* msg = &g_chat_messages[g_chat_message_count];
                    msg->sender_id = senderId;
                    strncpy(msg->message, msg_content, sizeof(msg->message) - 1);
                    msg->message[sizeof(msg->message) - 1] = '\0';
                    strcpy(msg->time, "Just now");
                    msg->is_me = false; // Message from others
                    g_chat_message_count++;
                    g_should_scroll_to_bottom = true; // Auto-scroll when receiving new message
                    printf("ChatScreen: Received message from %ld\n", senderId);
                }
            }
        }
    }
    free(msg_copy);
}

void drawChatListPanel()
{
    static bool isInitialized = false;
    if (!isInitialized) {
        // One-time initialization for this screen
        Network_set_async_message_handler(handle_async_messages);
        
        // Get the current user ID from auth service
        g_my_user_id = AuthService_get_current_user_id();
        printf("ChatScreen: Initialized with user ID: %ld\n", g_my_user_id);

        int count = 0;
        long* contacts = MessageService_get_contacts(&count);
        if (contacts != NULL) {
            g_contact_count = count < 1024 ? count : 1024;
            memcpy(g_contact_ids, contacts, g_contact_count * sizeof(long));
            free(contacts);
        }
        isInitialized = true;
    }

    // --- Drawing Code (Your original UI) ---
    Rectangle Panel_ChatList = { 0, 0, WINDOW_SCREEN_WIDTH/4, WINDOW_SCREEN_HEIGHT };
    Rectangle Panel_ChatListHeader = { Panel_ChatList.x, Panel_ChatList.y, Panel_ChatList.width, 180 };
    DrawRectangleRec(Panel_ChatList, COLOR_DARKTHEME_BLACK);
    DrawRectangleRec(Panel_ChatListHeader, COLOR_DARKTHEME_BLACK);
    // DrawTexture(Texture_ChatListIcon, 0, 0, WHITE); // Assuming loadResizeImage is a custom function

    Vector2 Measure_ChatListTitle = MeasureTextEx(Font_Opensans_Bold_20, WINDOW_TITLE, 20, 1);
    Vector2 Position_ChatListTitle = { Panel_ChatList.width/2 - Measure_ChatListTitle.x/2, 30 - 12 };
    DrawTextEx(Font_Opensans_Bold_20, WINDOW_TITLE, Position_ChatListTitle, 20, 1, WHITE);

    Rectangle Position_NewChatButton = { Panel_ChatList.width/2 - 85, 60, 170, 40 };
    char Text_NewChatTitle[] = "New Chat";
    DrawRoundedButton(CreateRoundedButton(Position_NewChatButton, COLOR_DARKTHEME_PURPLE, COLOR_DARKTHEME_BLACK, COLOR_DARKTHEME_GRAY, Text_NewChatTitle, &Font_Opensans_Bold_17, 15, 0));

    Rectangle Position_JoinRoomButton = { Panel_ChatList.width/2 - 85, 110, 170, 40 };
    char Text_JoinRoomTitle[] = "Join Room";
    DrawRoundedButton(CreateRoundedButton(Position_JoinRoomButton, COLOR_DARKTHEME_PURPLE, COLOR_DARKTHEME_BLACK, COLOR_DARKTHEME_GRAY, Text_JoinRoomTitle, &Font_Opensans_Bold_17, 15, 0));

    Rectangle Position_GuiListView = { 0, 180, Panel_ChatList.width, 420 };

    float numberOfItems = g_contact_count;
    float itemHeight = 40;
    static float scrollY = 0.0f;
    static float scrollSpeed = 20.0f;
    float contentHeight = numberOfItems * itemHeight;
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, Position_GuiListView))
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            scrollY += wheel * scrollSpeed;
            if (scrollY > 0) scrollY = 0;
            float maxScroll = contentHeight - Position_GuiListView.height;
            if (contentHeight > Position_GuiListView.height && scrollY < -maxScroll) scrollY = -maxScroll;
        }
    }

    BeginScissorMode((int)Position_GuiListView.x, (int)Position_GuiListView.y, (int)Position_GuiListView.width, (int)Position_GuiListView.height);
    for (int i = 0; i < numberOfItems; i++)
    {
        float itemY = Position_GuiListView.y + scrollY + (i * itemHeight);
        if (itemY + itemHeight < Position_GuiListView.y || itemY > Position_GuiListView.y + Position_GuiListView.height) continue;

        Rectangle Position_ChatListButton = { 0, itemY, Panel_ChatList.width, 40 };

        // Click handling
        if (CheckCollisionPointRec(mousePos, Position_ChatListButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            g_current_chat_contact_id = g_contact_ids[i];
            char* history_str = MessageService_get_history(g_current_chat_contact_id);
            parse_and_load_messages(history_str);
            if (history_str) free(history_str);
        }

        char user_text[64];
        snprintf(user_text, sizeof(user_text), "User ID: %ld", g_contact_ids[i]);
        ChatListButton(Position_ChatListButton, user_text, Font_Opensans_Bold_17, 15, COLOR_DARKTHEME_GRAY, COLOR_DARKTHEME_BLACK, COLOR_DARKTHEME_GRAY, WHITE, 0);
    }
    EndScissorMode();
}

void drawChatSection()
{
    Rectangle Position_ChatSection = { WINDOW_SCREEN_WIDTH/4, 0, WINDOW_SCREEN_WIDTH/4*3, WINDOW_SCREEN_HEIGHT };
    DrawRectangleRec(Position_ChatSection, COLOR_DARKTHEME_GRAY);

    Rectangle Position_ChatName = { Position_ChatSection.x, Position_ChatSection.y, Position_ChatSection.width, 50 };
    DrawRectangleRec(Position_ChatName, COLOR_DARKTHEME_BLACK);
    Vector2 Position_ChatNameText = { Position_ChatName.x + 20, Position_ChatName.y + 15 };

    if (g_current_chat_contact_id != -1) {
        DrawTextEx(Font_Opensans_Bold_20, TextFormat("Chat with User %ld", g_current_chat_contact_id), Position_ChatNameText, 20, 0, WHITE);
    } else {
        DrawTextEx(Font_Opensans_Bold_20, "Select a chat", Position_ChatNameText, 20, 0, WHITE);
    }

    Rectangle Position_ChatPage = { 200, Position_ChatName.y + Position_ChatName.height, 600, Position_ChatSection.height - Position_ChatName.height - 40 };
    DrawRectangleRec(Position_ChatPage, COLOR_DARKTHEME_GRAY);

    float numberOfItems = g_chat_message_count;
    float itemHeight = 80;
    static float scrollY = 0.0f;
    static float scrollSpeed = 20.0f;
    float contentHeight = numberOfItems * itemHeight;
    float maxScroll = contentHeight - Position_ChatPage.height;
    if (maxScroll < 0) maxScroll = 0;
    
    // Auto-scroll to bottom when new messages arrive
    if (g_should_scroll_to_bottom) {
        if (contentHeight > Position_ChatPage.height) {
            scrollY = -maxScroll;
        } else {
            scrollY = 0;
        }
        g_should_scroll_to_bottom = false;
    }
    
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, Position_ChatPage))
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            scrollY += wheel * scrollSpeed;
            if (scrollY > 0) scrollY = 0;
            if (contentHeight > Position_ChatPage.height && scrollY < -maxScroll) scrollY = -maxScroll;
        }
    }

    BeginScissorMode((int)Position_ChatPage.x, (int)Position_ChatPage.y, (int)Position_ChatPage.width, (int)Position_ChatPage.height);
    for (int i = 0; i < numberOfItems; i++)
    {
        float itemY = Position_ChatPage.y + scrollY + (i * itemHeight);
        if (itemY + itemHeight < Position_ChatPage.y || itemY > Position_ChatPage.y + Position_ChatPage.height) continue;

        Rectangle Position_bubbleChat = { Position_ChatPage.x, itemY, Position_ChatPage.width, 40 };

        // Use is_me from message to determine bubble position (left/right)
        DrawBubbleChat(Position_bubbleChat, g_chat_messages[i].message, Font_Opensans_Regular_20, 20, BLACK, g_chat_messages[i].is_me);
    }
    EndScissorMode();

    // Draw the input box
    Rectangle Position_InputBox = {
        210,
        550,
        580,
        40
    };
    static char dynamic_chatsceen_input[100] = "";
    static bool dynamic_chatscreen_isActive = false;
    TextField inputBox = createTextField(
        "Type your message here...",
        dynamic_chatsceen_input,
        &dynamic_chatscreen_isActive,
        Position_InputBox,
        0.3f,
        10.0f,
        &Font_Opensans_Regular_20
    );
    drawTextField(&inputBox);

    // Handle sending message when Enter is pressed
    if (dynamic_chatscreen_isActive && IsKeyPressed(KEY_ENTER)) {
        if (g_current_chat_contact_id != -1 && strlen(dynamic_chatsceen_input) > 0) {
            // Save message content before sending (in case input gets modified)
            char message_to_send[100];
            strncpy(message_to_send, dynamic_chatsceen_input, sizeof(message_to_send) - 1);
            message_to_send[sizeof(message_to_send) - 1] = '\0';
            
            // Send the message via MessageService
            bool send_success = MessageService_send_dm(g_current_chat_contact_id, message_to_send);
            
            if (send_success && g_chat_message_count < 2048) {
                // Add the sent message to local chat display
                ChatMessage* msg = &g_chat_messages[g_chat_message_count];
                msg->sender_id = g_my_user_id;
                strncpy(msg->message, message_to_send, sizeof(msg->message) - 1);
                msg->message[sizeof(msg->message) - 1] = '\0';
                strcpy(msg->time, "Just now");
                msg->is_me = true;
                g_chat_message_count++;
                g_should_scroll_to_bottom = true; // Auto-scroll to show sent message
                
                printf("ChatScreen: Message sent successfully.\n");
            } else if (!send_success) {
                printf("ChatScreen: Failed to send message.\n");
            }
            
            // Clear input field after sending
            memset(dynamic_chatsceen_input, 0, sizeof(dynamic_chatsceen_input));
        }
    }
}

void drawChatScreen()
{
    drawChatListPanel();
    drawChatSection();
}
