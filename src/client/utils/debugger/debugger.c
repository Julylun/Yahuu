#include "debugger.h"
#include <raylib.h>


bool g_isDebugModeEnabled = false;
bool isDebugModeEnabled() {
    return g_isDebugModeEnabled;
}
void toggleDebugMode() {
    g_isDebugModeEnabled = !g_isDebugModeEnabled;
}

static char *debugMessage;
int debugging_font_size = 15;
void setDebugMessage(const char *message) {
    debugMessage = message;
}


void drawDebugInfo() {
    if (isDebugModeEnabled()) {
        DrawText("[DEBUG MODE]", 10, 10, debugging_font_size, YELLOW);
        DrawText(TextFormat("Current FPS: %f", GetFPS()), 10, 12 + debugging_font_size, debugging_font_size, WHITE);
        DrawText(TextFormat("Current State: %s", getScreenStateName(getScreenState())), 10, 15 + 2 * debugging_font_size, debugging_font_size, WHITE);
        if (debugMessage != 0)
            DrawText(debugMessage, 10, 18 + 3 * debugging_font_size, debugging_font_size, WHITE);
    }
}
