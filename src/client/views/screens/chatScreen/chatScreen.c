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
    char sender_name[256]; // Username of sender (for room messages)
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

// Room/Group state
typedef struct {
    long id;
    char name[256];
} RoomInfo;

static RoomInfo g_rooms[256];
static int g_room_count = 0;
static bool g_is_current_chat_room = false; // true if chatting in a room, false for DM

// Room Dialog state
static bool g_show_room_dialog = false;
static int g_room_dialog_mode = 0; // 0 = menu, 1 = create, 2 = join
static char g_room_input[256] = "";
static bool g_room_input_active = false;
static bool g_room_action_done = false;
static bool g_room_action_success = false;
static char g_room_action_message[256] = "";

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
            msg->sender_name[0] = '\0'; // Will be populated later for rooms
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

// Helper to get username by ID (with caching)
static void get_username_by_id(long userId, char* out_name, int buffer_size) {
    if (out_name == NULL || buffer_size <= 0) return;
    
    // Check if it's me
    if (userId == g_my_user_id) {
        strncpy(out_name, "Me", buffer_size - 1);
        out_name[buffer_size - 1] = '\0';
        return;
    }
    
    // Check in contacts cache
    for (int i = 0; i < g_contact_count; i++) {
        if (g_contacts[i].id == userId) {
            strncpy(out_name, g_contacts[i].username, buffer_size - 1);
            out_name[buffer_size - 1] = '\0';
            return;
        }
    }
    
    // Not in cache, fetch from server
    if (!MessageService_get_user_info(userId, out_name, buffer_size)) {
        snprintf(out_name, buffer_size, "User %ld", userId);
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

                if (!g_is_current_chat_room && senderId == g_current_chat_contact_id && g_chat_message_count < 2048) {
                    ChatMessage* msg = &g_chat_messages[g_chat_message_count];
                    msg->sender_id = senderId;
                    strncpy(msg->message, msg_content, sizeof(msg->message) - 1);
                    msg->message[sizeof(msg->message) - 1] = '\0';
                    strcpy(msg->time, "Just now");
                    msg->is_me = false; // Message from others
                    msg->sender_name[0] = '\0'; // Not needed for DM
                    g_chat_message_count++;
                    g_should_scroll_to_bottom = true;
                    printf("ChatScreen: Received DM from %ld\n", senderId);
                }
            }
        } else if (strcmp(command, "RECEIVE_GROUP_MSG") == 0) {
            char* groupId_str = strtok(NULL, separator);
            char* senderId_str = strtok(NULL, separator);
            char* msg_content = strtok(NULL, separator);
            if (groupId_str && senderId_str && msg_content) {
                long groupId = atol(groupId_str);
                long senderId = atol(senderId_str);

                // If we're currently viewing this room, add the message
                if (g_is_current_chat_room && groupId == g_current_chat_contact_id && g_chat_message_count < 2048) {
                    ChatMessage* msg = &g_chat_messages[g_chat_message_count];
                    msg->sender_id = senderId;
                    strncpy(msg->message, msg_content, sizeof(msg->message) - 1);
                    msg->message[sizeof(msg->message) - 1] = '\0';
                    strcpy(msg->time, "Just now");
                    msg->is_me = (senderId == g_my_user_id);
                    // Get sender username for room message
                    get_username_by_id(senderId, msg->sender_name, sizeof(msg->sender_name));
                    g_chat_message_count++;
                    g_should_scroll_to_bottom = true;
                    printf("ChatScreen: Received group message in room %ld from %ld\n", groupId, senderId);
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

        // Load contacts (DMs)
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
        
        // Load rooms/groups
        long room_ids[256];
        char room_names[256][256];
        g_room_count = MessageService_get_my_groups(room_ids, room_names, 256);
        for (int i = 0; i < g_room_count; i++) {
            g_rooms[i].id = room_ids[i];
            strncpy(g_rooms[i].name, room_names[i], sizeof(g_rooms[i].name) - 1);
            g_rooms[i].name[sizeof(g_rooms[i].name) - 1] = '\0';
        }
        printf("ChatScreen: Loaded %d rooms.\n", g_room_count);
        
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
    char Text_JoinRoomTitle[] = "Rooms";
    
    // Check if Join Room button is clicked
    if (CheckCollisionPointRec(mousePos_btn, Position_JoinRoomButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        g_show_room_dialog = true;
        g_room_dialog_mode = 0; // Show menu
        g_room_action_done = false;
        memset(g_room_input, 0, sizeof(g_room_input));
    }
    
    DrawRoundedButton(CreateRoundedButton(Position_JoinRoomButton, COLOR_DARKTHEME_PURPLE, COLOR_DARKTHEME_BLACK, COLOR_DARKTHEME_GRAY, Text_JoinRoomTitle, &Font_Opensans_Bold_17, 15, 0));

    Rectangle Position_GuiListView = { 0, 180, Panel_ChatList.width, 420 };

    // Total items = contacts + rooms
    float numberOfItems = g_contact_count + g_room_count;
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
    
    int itemIndex = 0;
    
    // Draw rooms first (with "Room -> " prefix)
    for (int i = 0; i < g_room_count; i++)
    {
        float itemY = Position_GuiListView.y + scrollY + (itemIndex * itemHeight);
        if (itemY + itemHeight >= Position_GuiListView.y && itemY <= Position_GuiListView.y + Position_GuiListView.height) {
            Rectangle Position_ChatListButton = { 0, itemY, Panel_ChatList.width, 40 };

            // Click handling for room
            if (CheckCollisionPointRec(mousePos, Position_ChatListButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                g_current_chat_contact_id = g_rooms[i].id;
                snprintf(g_current_chat_contact_name, sizeof(g_current_chat_contact_name), "Room -> %s", g_rooms[i].name);
                g_is_current_chat_room = true;
                char* history_str = MessageService_get_group_history(g_current_chat_contact_id);
                parse_and_load_messages(history_str);
                if (history_str) free(history_str);
                
                // Fetch sender names for room messages
                for (int j = 0; j < g_chat_message_count; j++) {
                    get_username_by_id(g_chat_messages[j].sender_id, g_chat_messages[j].sender_name, sizeof(g_chat_messages[j].sender_name));
                }
            }

            char room_display[300];
            snprintf(room_display, sizeof(room_display), "Room -> %s", g_rooms[i].name);
            ChatListButton(Position_ChatListButton, room_display, Font_Opensans_Bold_17, 15, (Color){60, 60, 80, 255}, COLOR_DARKTHEME_BLACK, COLOR_DARKTHEME_GRAY, (Color){150, 200, 255, 255}, 0);
        }
        itemIndex++;
    }
    
    // Draw contacts (DMs)
    for (int i = 0; i < g_contact_count; i++)
    {
        float itemY = Position_GuiListView.y + scrollY + (itemIndex * itemHeight);
        if (itemY + itemHeight >= Position_GuiListView.y && itemY <= Position_GuiListView.y + Position_GuiListView.height) {
            Rectangle Position_ChatListButton = { 0, itemY, Panel_ChatList.width, 40 };

            // Click handling for contact
            if (CheckCollisionPointRec(mousePos, Position_ChatListButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                g_current_chat_contact_id = g_contacts[i].id;
                strncpy(g_current_chat_contact_name, g_contacts[i].username, sizeof(g_current_chat_contact_name) - 1);
                g_current_chat_contact_name[sizeof(g_current_chat_contact_name) - 1] = '\0';
                g_is_current_chat_room = false;
                char* history_str = MessageService_get_history(g_current_chat_contact_id);
                parse_and_load_messages(history_str);
                if (history_str) free(history_str);
            }

            ChatListButton(Position_ChatListButton, g_contacts[i].username, Font_Opensans_Bold_17, 15, COLOR_DARKTHEME_GRAY, COLOR_DARKTHEME_BLACK, COLOR_DARKTHEME_GRAY, WHITE, 0);
        }
        itemIndex++;
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

        Rectangle Position_bubbleChat = { Position_ChatPage.x, itemY + 20, Position_ChatPage.width, 40 };

        // For room messages, show sender name above the bubble
        if (g_is_current_chat_room && strlen(g_chat_messages[i].sender_name) > 0) {
            Color nameColor = g_chat_messages[i].is_me ? (Color){150, 200, 255, 255} : (Color){255, 200, 150, 255};
            Vector2 namePos;
            if (g_chat_messages[i].is_me) {
                // Align right for my messages
                Vector2 nameSize = MeasureTextEx(Font_Opensans_Regular_20, g_chat_messages[i].sender_name, 14, 1);
                namePos = (Vector2){ Position_ChatPage.x + Position_ChatPage.width - nameSize.x - 30, itemY + 2 };
            } else {
                // Align left for others
                namePos = (Vector2){ Position_ChatPage.x + 30, itemY + 2 };
            }
            DrawTextEx(Font_Opensans_Regular_20, g_chat_messages[i].sender_name, namePos, 14, 1, nameColor);
        }

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
            
            bool send_success = false;
            
            // Send via appropriate service based on chat type
            if (g_is_current_chat_room) {
                send_success = MessageService_send_group_message(g_current_chat_contact_id, message_to_send);
            } else {
                send_success = MessageService_send_dm(g_current_chat_contact_id, message_to_send);
            }
            
            if (send_success && g_chat_message_count < 2048) {
                // Add the sent message to local chat display
                ChatMessage* msg = &g_chat_messages[g_chat_message_count];
                msg->sender_id = g_my_user_id;
                strncpy(msg->message, message_to_send, sizeof(msg->message) - 1);
                msg->message[sizeof(msg->message) - 1] = '\0';
                strcpy(msg->time, "Just now");
                msg->is_me = true;
                // Set sender name for room messages
                if (g_is_current_chat_room) {
                    strcpy(msg->sender_name, "Me");
                } else {
                    msg->sender_name[0] = '\0';
                }
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

static void addRoomToList(long roomId, const char* roomName) {
    // Check if room already exists
    for (int i = 0; i < g_room_count; i++) {
        if (g_rooms[i].id == roomId) return;
    }
    if (g_room_count < 256) {
        g_rooms[g_room_count].id = roomId;
        strncpy(g_rooms[g_room_count].name, roomName, sizeof(g_rooms[g_room_count].name) - 1);
        g_rooms[g_room_count].name[sizeof(g_rooms[g_room_count].name) - 1] = '\0';
        g_room_count++;
    }
}

static void drawRoomDialog()
{
    if (!g_show_room_dialog) return;
    
    // Draw semi-transparent overlay
    DrawRectangle(0, 0, WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, ColorAlpha(BLACK, 0.7f));
    
    // Dialog box
    float dialogWidth = 400;
    float dialogHeight = 300;
    Rectangle dialogRect = {
        WINDOW_SCREEN_WIDTH / 2 - dialogWidth / 2,
        WINDOW_SCREEN_HEIGHT / 2 - dialogHeight / 2,
        dialogWidth,
        dialogHeight
    };
    DrawRectangleRounded(dialogRect, 0.1f, 10, COLOR_DARKTHEME_GRAY);
    
    // Title
    const char* title = "Rooms";
    Vector2 titleSize = MeasureTextEx(Font_Opensans_Bold_20, title, 20, 1);
    Vector2 titlePos = { dialogRect.x + dialogRect.width / 2 - titleSize.x / 2, dialogRect.y + 20 };
    DrawTextEx(Font_Opensans_Bold_20, title, titlePos, 20, 1, WHITE);
    
    Vector2 mousePos = GetMousePosition();
    
    if (g_room_dialog_mode == 0) {
        // Menu mode - show Create and Join buttons
        Rectangle createBtn = { dialogRect.x + 50, dialogRect.y + 80, dialogRect.width - 100, 40 };
        Rectangle joinBtn = { dialogRect.x + 50, dialogRect.y + 140, dialogRect.width - 100, 40 };
        
        bool createHover = CheckCollisionPointRec(mousePos, createBtn);
        bool joinHover = CheckCollisionPointRec(mousePos, joinBtn);
        
        DrawRectangleRounded(createBtn, 0.3f, 10, createHover ? COLOR_DARKTHEME_BLACK : COLOR_DARKTHEME_PURPLE);
        DrawRectangleRounded(joinBtn, 0.3f, 10, joinHover ? COLOR_DARKTHEME_BLACK : COLOR_DARKTHEME_PURPLE);
        
        Vector2 createTextSize = MeasureTextEx(Font_Opensans_Bold_17, "Create New Room", 17, 1);
        Vector2 joinTextSize = MeasureTextEx(Font_Opensans_Bold_17, "Join Room by ID", 17, 1);
        
        DrawTextEx(Font_Opensans_Bold_17, "Create New Room", 
            (Vector2){ createBtn.x + createBtn.width/2 - createTextSize.x/2, createBtn.y + createBtn.height/2 - createTextSize.y/2 }, 
            17, 1, WHITE);
        DrawTextEx(Font_Opensans_Bold_17, "Join Room by ID", 
            (Vector2){ joinBtn.x + joinBtn.width/2 - joinTextSize.x/2, joinBtn.y + joinBtn.height/2 - joinTextSize.y/2 }, 
            17, 1, WHITE);
        
        if (createHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            g_room_dialog_mode = 1;
            g_room_action_done = false;
            memset(g_room_input, 0, sizeof(g_room_input));
        }
        if (joinHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            g_room_dialog_mode = 2;
            g_room_action_done = false;
            memset(g_room_input, 0, sizeof(g_room_input));
        }
        
    } else if (g_room_dialog_mode == 1) {
        // Create room mode
        DrawTextEx(Font_Opensans_Regular_20, "Enter room name:", (Vector2){ dialogRect.x + 30, dialogRect.y + 70 }, 18, 1, WHITE);
        
        Rectangle inputRect = { dialogRect.x + 30, dialogRect.y + 100, dialogRect.width - 60, 35 };
        TextField inputField = createTextField("Room name...", g_room_input, &g_room_input_active, inputRect, 0.3f, 10.0f, &Font_Opensans_Regular_20);
        drawTextField(&inputField);
        
        // Create button
        Rectangle actionBtn = { dialogRect.x + dialogRect.width/2 - 80, dialogRect.y + 150, 160, 35 };
        bool actionHover = CheckCollisionPointRec(mousePos, actionBtn);
        DrawRectangleRounded(actionBtn, 0.3f, 10, actionHover ? COLOR_DARKTHEME_BLACK : COLOR_DARKTHEME_PURPLE);
        Vector2 actionTextSize = MeasureTextEx(Font_Opensans_Bold_17, "Create", 17, 1);
        DrawTextEx(Font_Opensans_Bold_17, "Create", 
            (Vector2){ actionBtn.x + actionBtn.width/2 - actionTextSize.x/2, actionBtn.y + actionBtn.height/2 - actionTextSize.y/2 }, 
            17, 1, WHITE);
        
        if ((actionHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || (g_room_input_active && IsKeyPressed(KEY_ENTER))) {
            if (strlen(g_room_input) > 0) {
                long newRoomId = -1;
                g_room_action_success = MessageService_create_group(g_room_input, &newRoomId);
                if (g_room_action_success && newRoomId > 0) {
                    snprintf(g_room_action_message, sizeof(g_room_action_message), "Room created! ID: %ld", newRoomId);
                    addRoomToList(newRoomId, g_room_input);
                } else {
                    snprintf(g_room_action_message, sizeof(g_room_action_message), "Failed to create room");
                }
                g_room_action_done = true;
            }
        }
        
        // Back button
        Rectangle backBtn = { dialogRect.x + 30, dialogRect.y + dialogRect.height - 50, 80, 30 };
        bool backHover = CheckCollisionPointRec(mousePos, backBtn);
        DrawTextEx(Font_Opensans_Regular_20, "< Back", (Vector2){ backBtn.x, backBtn.y + 5 }, 16, 1, backHover ? (Color){255, 200, 200, 255} : WHITE);
        if (backHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            g_room_dialog_mode = 0;
            g_room_action_done = false;
        }
        
        // Show result
        if (g_room_action_done) {
            Color resultColor = g_room_action_success ? (Color){100, 255, 100, 255} : (Color){255, 100, 100, 255};
            DrawTextEx(Font_Opensans_Regular_20, g_room_action_message, (Vector2){ dialogRect.x + 30, dialogRect.y + 200 }, 16, 1, resultColor);
        }
        
    } else if (g_room_dialog_mode == 2) {
        // Join room mode
        DrawTextEx(Font_Opensans_Regular_20, "Enter room ID:", (Vector2){ dialogRect.x + 30, dialogRect.y + 70 }, 18, 1, WHITE);
        
        Rectangle inputRect = { dialogRect.x + 30, dialogRect.y + 100, dialogRect.width - 60, 35 };
        TextField inputField = createTextField("Room ID (number)...", g_room_input, &g_room_input_active, inputRect, 0.3f, 10.0f, &Font_Opensans_Regular_20);
        drawTextField(&inputField);
        
        // Join button
        Rectangle actionBtn = { dialogRect.x + dialogRect.width/2 - 80, dialogRect.y + 150, 160, 35 };
        bool actionHover = CheckCollisionPointRec(mousePos, actionBtn);
        DrawRectangleRounded(actionBtn, 0.3f, 10, actionHover ? COLOR_DARKTHEME_BLACK : COLOR_DARKTHEME_PURPLE);
        Vector2 actionTextSize = MeasureTextEx(Font_Opensans_Bold_17, "Join", 17, 1);
        DrawTextEx(Font_Opensans_Bold_17, "Join", 
            (Vector2){ actionBtn.x + actionBtn.width/2 - actionTextSize.x/2, actionBtn.y + actionBtn.height/2 - actionTextSize.y/2 }, 
            17, 1, WHITE);
        
        if ((actionHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || (g_room_input_active && IsKeyPressed(KEY_ENTER))) {
            if (strlen(g_room_input) > 0) {
                long roomId = atol(g_room_input);
                char roomName[256] = "";
                g_room_action_success = MessageService_join_group(roomId, roomName, sizeof(roomName));
                if (g_room_action_success) {
                    snprintf(g_room_action_message, sizeof(g_room_action_message), "Joined room: %s", roomName);
                    addRoomToList(roomId, roomName);
                } else {
                    snprintf(g_room_action_message, sizeof(g_room_action_message), "Failed to join (not found or already member)");
                }
                g_room_action_done = true;
            }
        }
        
        // Back button
        Rectangle backBtn = { dialogRect.x + 30, dialogRect.y + dialogRect.height - 50, 80, 30 };
        bool backHover = CheckCollisionPointRec(mousePos, backBtn);
        DrawTextEx(Font_Opensans_Regular_20, "< Back", (Vector2){ backBtn.x, backBtn.y + 5 }, 16, 1, backHover ? (Color){255, 200, 200, 255} : WHITE);
        if (backHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            g_room_dialog_mode = 0;
            g_room_action_done = false;
        }
        
        // Show result
        if (g_room_action_done) {
            Color resultColor = g_room_action_success ? (Color){100, 255, 100, 255} : (Color){255, 100, 100, 255};
            DrawTextEx(Font_Opensans_Regular_20, g_room_action_message, (Vector2){ dialogRect.x + 30, dialogRect.y + 200 }, 16, 1, resultColor);
        }
    }
    
    // Close button (X)
    Rectangle closeBtn = { dialogRect.x + dialogRect.width - 35, dialogRect.y + 10, 25, 25 };
    bool closeHover = CheckCollisionPointRec(mousePos, closeBtn);
    DrawTextEx(Font_Opensans_Bold_20, "X", (Vector2){ closeBtn.x + 5, closeBtn.y + 2 }, 20, 1, closeHover ? RED : WHITE);
    
    if (closeHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        g_show_room_dialog = false;
        g_room_dialog_mode = 0;
    }
    
    if (IsKeyPressed(KEY_ESCAPE)) {
        g_show_room_dialog = false;
        g_room_dialog_mode = 0;
    }
}

void drawChatScreen()
{
    drawChatListPanel();
    drawChatSection();
    drawNewChatDialog();
    drawRoomDialog(); // Draw room dialog on top
}
