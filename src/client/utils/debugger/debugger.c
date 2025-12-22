#include "debugger.h"


bool g_isDebugModeEnabled = false;
bool isDebugModeEnabled() {
    return g_isDebugModeEnabled;
}
void toggleDebugMode() {
    g_isDebugModeEnabled = !g_isDebugModeEnabled;
}
void drawDebugInfo() {
    if (isDebugModeEnabled()) {
        DrawText("[DEBUG MODE]", 10, 10, 20, YELLOW);
        DrawText(TextFormat("Current FPS: %f", GetFPS()), 10, 40, 20, WHITE);
        DrawText(TextFormat("Current State: %s", getScreenStateName(getScreenState())), 10, 70, 20, WHITE);
    }
}
