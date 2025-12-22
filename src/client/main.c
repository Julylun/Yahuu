#include <raylib.h>
#include <stdio.h>
#include "utils/constants/constants.h"
#include "utils/fonts/fonts.h"
#include "views/screenManager/screenManager.h"
#include "services/networkService/networkService.h" // Include the network service
#include "utils/debugger/debugger.h"

int main()
{
    // --- Basic App Init ---
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, WINDOW_TITLE);
    loadFonts();
    InitAudioDevice();
    SetTargetFPS(WINDOW_FRAME);
    toggleDebugMode();

    // --- Network Init ---
    setDebugMessage("Attempting to connect to the server...\n");
    if (Network_connect("127.0.0.1", 8080) < 0) {
        // Handle connection failure if necessary
        setDebugMessage("Could not connect to the server. Running in offline mode.\n");
    } else {
        // If connection is successful, start the listener thread
        Network_start_listener();
        setDebugMessage("Connected to server");
    }

    // Main game loop
    while (!WindowShouldClose())
    {
        // --- Update logic ---
        updateScreen(); // Update your UI/game state

        // A simple test to send a message to the server when you press ENTER
        if (IsKeyPressed(KEY_ENTER))
        {
            printf("Sending a test message to the server...\n");
            Network_send("Hello from the Raylib client!");
        }

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
