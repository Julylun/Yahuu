#include "screenManager.h"


void updateScreen()
{
    enum ScreenState g_screenState = getScreenState();

    switch (g_screenState) {

        case MAIN_MENU:
            break;
        case LOGIN:
            drawLoginScreen();
            break;
        case REGISTER:
            drawRegisterScreen();
            break;
    }
    drawDebugInfo();
}
