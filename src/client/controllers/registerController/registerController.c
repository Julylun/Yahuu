#include "registerController.h"

bool Register_onClickRegisterButton(const char *username,const char *password) {
    bool status = AuthService_register(username, password);

    if (status) {
        changeScreenState(LOGIN);
        setDebugMessage("Signup Sucessfully");
        return true;
    } else {
        setDebugMessage("Signup Failed");
        return false;
    }
}

void Register_onClickIhaveAccount() {
    changeScreenState(LOGIN);
}
