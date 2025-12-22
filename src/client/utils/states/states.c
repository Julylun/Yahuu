#include "states.h"

enum ScreenState g_screenState = LOGIN;

void changeScreenState(enum ScreenState newState)
{
    g_screenState = newState;
}

enum ScreenState getScreenState()
{
    return g_screenState;
}

char *getScreenStateName(enum ScreenState state)
{
    switch (state)
    {
        case LOGIN:
            return "LOGIN";
        case REGISTER:
            return "REGISTER";
        case MAIN_MENU:
            return "MAIN_MENU";
       default:
            return "UNKNOWN";
    }
}
