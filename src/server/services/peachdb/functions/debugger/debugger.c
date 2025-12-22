#include "debugger.h"

bool Debug_g_debugging = false;
void Debug_info(const char* message) {
    printf(ANSI_COLOR_GREEN "[INFO] %s\n" ANSI_COLOR_RESET, message);
}

void Debug_error(const char* message) {
    printf(ANSI_COLOR_RED "[ERROR] %s\n" ANSI_COLOR_RESET, message);
}

void Debug_warning(const char* message) {
    printf(ANSI_COLOR_YELLOW "[WARNING] %s\n" ANSI_COLOR_RESET, message);
}

void Debug_log(const char* message) {
    printf(ANSI_COLOR_BLUE "[LOG] %s\n" ANSI_COLOR_RESET, message);
}

void Debug_debug(const char* message) {
    if (Debug_g_debugging)
        printf(ANSI_COLOR_MAGENTA "[DEBUG] %s\n" ANSI_COLOR_RESET, message);
}

bool Debug_is_debug() {
    return Debug_g_debugging;
}

void Debug_set_debug(bool value) {
    Debug_g_debugging = value;
}

void Debug_init() {
    Debug_g_debugging = false;
}
