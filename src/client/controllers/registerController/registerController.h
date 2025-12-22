#ifndef REGISTER_CONTROLLER_H
#define REGISTER_CONTROLLER_H

#include "../../utils/states/states.h"
#include "../../services/authService/authService.h"
#include "../../utils/debugger/debugger.h"
#include <stdbool.h>


bool Register_onClickRegisterButton(const char *username,const char *password);
void Register_onClickIhaveAccount();

#endif
