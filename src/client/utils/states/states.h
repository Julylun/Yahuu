#ifndef STATES_H
#define STATES_H

enum ScreenState {
    CHAT,
    LOGIN,
    REGISTER,
};

void changeScreenState(enum ScreenState newState);
enum ScreenState getScreenState();
char *getScreenStateName(enum ScreenState state);

#endif
