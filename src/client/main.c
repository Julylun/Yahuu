#include <raylib.h>
#include "utils/constants/constants.h"
#include "utils/fonts/fonts.h"
#include "views/screenManager/screenManager.h"
#include "utils/debugger/debugger.h"

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_SCREEN_WIDTH, WINDOW_SCREEN_HEIGHT, WINDOW_TITLE);
    loadFonts();
    InitAudioDevice();
    SetTargetFPS(WINDOW_FRAME);
    // toggleDebugMode();


    while (!WindowShouldClose())
    {
        BeginDrawing();
        updateScreen();
        EndDrawing();
    }
    CloseAudioDevice();
    CloseWindow();
}
