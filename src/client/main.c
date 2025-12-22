#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#define RAYGUI_IMPLEMENTATION
#include "views/raygui/raygui.h"
#include "utils/constants/constants.h"
#include "utils/fonts/fonts.h"
#include "views/screenManager/screenManager.h"
#include "services/networkService/networkService.h"
#include "services/messageService/messageService.h"
#include "services/groupService/groupService.h"     // Include group service
#include "views/screens/chatScreen/chatScreen.h" // Include the screen that handles chat logic
#include "utils/debugger/debugger.h"



int main()
{
    // --- Basic App Init ---
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, WINDOW_TITLE);
    loadFonts();
    InitAudioDevice();
    SetTargetFPS(WINDOW_FRAME);
    // toggleDebugMode();

    // --- Network Init ---
    setDebugMessage("Attempting to connect to the server...\n");
    if (Network_connect("127.0.0.1", 8080) < 0) {
        // Handle connection failure if necessary
        setDebugMessage("Could not connect to the server. Running in offline mode.\n");
    } else {
        // If connection is successful, start the listener thread
        Network_start_listener();
        setDebugMessage("Connected to server");

        // Initialize screen modules that need network services
        // ChatScreen_init();
    }

    // Main game loop
    while (!WindowShouldClose())
    {
        // --- Update logic ---
        updateScreen(); // Update your UI/game state

       // --- Drawing ---
        BeginDrawing();
        // Your existing updateScreen probably handles drawing,
        // but if not, drawing code goes here.
        // For this example, we'll just clear the background.
        // ClearBackground(RAYWHITE);
        // updateScreen();
        EndDrawing();
    }

    // --- De-Initialization ---
    printf("Shutting down...\n");
    CloseAudioDevice();     // Close the audio device
    CloseWindow();          // Close the raylib window
    Network_disconnect();   // Disconnect from the server

    return 0;
}
