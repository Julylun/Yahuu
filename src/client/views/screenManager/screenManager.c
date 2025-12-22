#include "screenManager.h"

void updateScreen()
{
    enum ScreenState g_screenState = getScreenState();

    switch (g_screenState) {

        case CHAT:
            drawChatScreen();
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
