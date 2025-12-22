#ifndef DEBUGGER_H
#define DEBUGGER_H
#include <stdio.h>
#include <stdbool.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

extern bool Debug_g_debugging;

bool Debug_is_debug();
void Debug_set_debug(bool isDebug);

void Debug_debug(const char* message);
void Debug_info(const char* message);
void Debug_error(const char* message);
void Debug_warning(const char* message);
void Debug_log(const char* message);
void Debug_init();

#endif
