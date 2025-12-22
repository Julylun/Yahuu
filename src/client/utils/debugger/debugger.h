#ifndef DEBUGGER_H
#define DEBUGGER_H
#include <raylib.h>
#include "../states/states.h"

bool isDebugModeEnabled();
void toggleDebugMode();
void drawDebugInfo();
void setDebugMessage(const char *message);

#endif
