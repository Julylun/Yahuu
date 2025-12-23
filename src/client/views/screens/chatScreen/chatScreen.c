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

// Contact info with username
typedef struct {
    long id;
    char username[256];
} ContactInfo;

static ContactInfo g_contacts[1024];
static int g_contact_count = 0;

static long g_current_chat_contact_id = -1;
static char g_current_chat_contact_name[256] = "";
static ChatMessage g_chat_messages[2048];
static int g_chat_message_count = 0;
static bool g_should_scroll_to_bottom = false; // Flag to auto-scroll when new messages arrive

// New Chat Dialog state
static bool g_show_new_chat_dialog = false;
static char g_new_chat_search_input[256] = "";
static bool g_new_chat_search_active = false;
static bool g_new_chat_search_done = false;
static bool g_new_chat_user_found = false;
static long g_new_chat_found_user_id = -1;
static char g_new_chat_found_username[256] = "";

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
        if (g_contacts[i].id == contactId) {
            found = true;
            break;
        }
    }
    if (!found && g_contact_count < 1024) {
        g_contacts[g_contact_count].id = contactId;
        // Try to fetch username
        if (!MessageService_get_user_info(contactId, g_contacts[g_contact_count].username, sizeof(g_contacts[g_contact_count].username))) {
            snprintf(g_contacts[g_contact_count].username, sizeof(g_contacts[g_contact_count].username), "User %ld", contactId);
        }
        g_contact_count++;
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
        long* contact_ids = MessageService_get_contacts(&count);
        if (contact_ids != NULL) {
            int num_contacts = count < 1024 ? count : 1024;
            for (int i = 0; i < num_contacts; i++) {
                g_contacts[i].id = contact_ids[i];
                // Fetch username for each contact
                if (!MessageService_get_user_info(contact_ids[i], g_contacts[i].username, sizeof(g_contacts[i].username))) {
                    snprintf(g_contacts[i].username, sizeof(g_contacts[i].username), "User %ld", contact_ids[i]);
                }
            }
            g_contact_count = num_contacts;
            free(contact_ids);
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
    
    // Check if New Chat button is clicked
    Vector2 mousePos_btn = GetMousePosition();
    if (CheckCollisionPointRec(mousePos_btn, Position_NewChatButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        g_show_new_chat_dialog = true;
        g_new_chat_search_done = false;
        g_new_chat_user_found = false;
        memset(g_new_chat_search_input, 0, sizeof(g_new_chat_search_input));
    }
    
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
            g_current_chat_contact_id = g_contacts[i].id;
            strncpy(g_current_chat_contact_name, g_contacts[i].username, sizeof(g_current_chat_contact_name) - 1);
            g_current_chat_contact_name[sizeof(g_current_chat_contact_name) - 1] = '\0';
            char* history_str = MessageService_get_history(g_current_chat_contact_id);
            parse_and_load_messages(history_str);
            if (history_str) free(history_str);
        }

        ChatListButton(Position_ChatListButton, g_contacts[i].username, Font_Opensans_Bold_17, 15, COLOR_DARKTHEME_GRAY, COLOR_DARKTHEME_BLACK, COLOR_DARKTHEME_GRAY, WHITE, 0);
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
        DrawTextEx(Font_Opensans_Bold_20, TextFormat("Chat with %s", g_current_chat_contact_name), Position_ChatNameText, 20, 0, WHITE);
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

static void drawNewChatDialog()
{
    if (!g_show_new_chat_dialog) return;
    
    // Draw semi-transparent overlay
    DrawRectangle(0, 0, WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, ColorAlpha(BLACK, 0.7f));
    
    // Dialog box
    float dialogWidth = 400;
    float dialogHeight = 250;
    Rectangle dialogRect = {
        WINDOW_SCREEN_WIDTH / 2 - dialogWidth / 2,
        WINDOW_SCREEN_HEIGHT / 2 - dialogHeight / 2,
        dialogWidth,
        dialogHeight
    };
    DrawRectangleRounded(dialogRect, 0.1f, 10, COLOR_DARKTHEME_GRAY);
    
    // Title
    const char* title = "New Chat";
    Vector2 titleSize = MeasureTextEx(Font_Opensans_Bold_20, title, 20, 1);
    Vector2 titlePos = { dialogRect.x + dialogRect.width / 2 - titleSize.x / 2, dialogRect.y + 20 };
    DrawTextEx(Font_Opensans_Bold_20, title, titlePos, 20, 1, WHITE);
    
    // Search label
    Vector2 labelPos = { dialogRect.x + 30, dialogRect.y + 60 };
    DrawTextEx(Font_Opensans_Regular_20, "Enter username:", labelPos, 18, 1, WHITE);
    
    // Search input field
    Rectangle searchInputRect = {
        dialogRect.x + 30,
        dialogRect.y + 90,
        dialogRect.width - 60,
        35
    };
    TextField searchField = createTextField(
        "Type username...",
        g_new_chat_search_input,
        &g_new_chat_search_active,
        searchInputRect,
        0.3f,
        10.0f,
        &Font_Opensans_Regular_20
    );
    drawTextField(&searchField);
    
    // Handle Enter key to search
    if (g_new_chat_search_active && IsKeyPressed(KEY_ENTER) && strlen(g_new_chat_search_input) > 0) {
        g_new_chat_user_found = MessageService_search_user(
            g_new_chat_search_input,
            &g_new_chat_found_user_id,
            g_new_chat_found_username,
            sizeof(g_new_chat_found_username)
        );
        g_new_chat_search_done = true;
    }
    
    // Show search result
    if (g_new_chat_search_done) {
        Vector2 resultPos = { dialogRect.x + 30, dialogRect.y + 135 };
        
        if (g_new_chat_user_found) {
            DrawTextEx(Font_Opensans_Regular_20, TextFormat("Found: %s", g_new_chat_found_username), resultPos, 18, 1, (Color){100, 255, 100, 255});
            
            // Start Chat button
            Rectangle startChatBtn = {
                dialogRect.x + dialogRect.width / 2 - 80,
                dialogRect.y + 165,
                160,
                35
            };
            
            Vector2 mousePos = GetMousePosition();
            bool btnHover = CheckCollisionPointRec(mousePos, startChatBtn);
            Color btnColor = btnHover ? COLOR_DARKTHEME_BLACK : COLOR_DARKTHEME_PURPLE;
            
            DrawRectangleRounded(startChatBtn, 0.3f, 10, btnColor);
            Vector2 btnTextSize = MeasureTextEx(Font_Opensans_Bold_17, "Start Chat", 17, 1);
            Vector2 btnTextPos = {
                startChatBtn.x + startChatBtn.width / 2 - btnTextSize.x / 2,
                startChatBtn.y + startChatBtn.height / 2 - btnTextSize.y / 2
            };
            DrawTextEx(Font_Opensans_Bold_17, "Start Chat", btnTextPos, 17, 1, WHITE);
            
            if (btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Add contact and start chat
                add_contact_if_not_exists(g_new_chat_found_user_id);
                g_current_chat_contact_id = g_new_chat_found_user_id;
                strncpy(g_current_chat_contact_name, g_new_chat_found_username, sizeof(g_current_chat_contact_name) - 1);
                g_chat_message_count = 0; // Clear messages for new chat
                g_show_new_chat_dialog = false;
            }
        } else {
            DrawTextEx(Font_Opensans_Regular_20, "User not found", resultPos, 18, 1, (Color){255, 100, 100, 255});
        }
    }
    
    // Close button (X)
    Rectangle closeBtn = { dialogRect.x + dialogRect.width - 35, dialogRect.y + 10, 25, 25 };
    Vector2 mousePos = GetMousePosition();
    bool closeHover = CheckCollisionPointRec(mousePos, closeBtn);
    
    DrawTextEx(Font_Opensans_Bold_20, "X", (Vector2){ closeBtn.x + 5, closeBtn.y + 2 }, 20, 1, closeHover ? RED : WHITE);
    
    if (closeHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        g_show_new_chat_dialog = false;
    }
    
    // Also close on ESC
    if (IsKeyPressed(KEY_ESCAPE)) {
        g_show_new_chat_dialog = false;
    }
}

void drawChatScreen()
{
    drawChatListPanel();
    drawChatSection();
    drawNewChatDialog(); // Draw dialog on top
}
