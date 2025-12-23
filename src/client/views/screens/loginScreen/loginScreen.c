#include "loginScreen.h"
#include <raylib.h>
#include <stdio.h>


static char dynamic_login_usernameValue[100] = "";
static bool dynamic_login_usernameIsActive = false;
static char dynamic_login_passwordValue[100] = "";
static bool dynamic_login_passwordIsActive = false;
void login()
{
    long userId = -1;
    bool status = AuthService_login(dynamic_login_usernameValue, dynamic_login_passwordValue, &userId);
    if (status) {
        printf("Login successful! User ID: %ld\n", userId);
        changeScreenState(CHAT);
    } else {
        setDebugMessage("Login failed!!!");
    }
}

void drawLoginScreen()
{
    // Init
    static Texture2D icon;
    static bool isInitialized = false;
    if (!isInitialized)
    {
        icon = loadResizeImage("assets/icon.png", 80, 80);
        isInitialized = true;
    }


    //# Draw components
    ClearBackground(COLOR_DARKTHEME_BLACK);

    //## Draw Panel
    const float LoginPanelWidth = 400.0f;
    const float LoginPanelHeight = 500.0f;
    const Rectangle LoginPanelShape = {
        WINDOW_SCREEN_WIDTH / 2 - LoginPanelWidth/2,
        WINDOW_SCREEN_HEIGHT / 2 - LoginPanelHeight/2,
        LoginPanelWidth,
        LoginPanelHeight
    };

    DrawRectangleRounded(LoginPanelShape,0.1f,3, ColorAlpha(COLOR_DARKTHEME_GRAY, 255.f));

    //## Draw Icon
    const float IconWidth = 60;
    const float IconHeight = 60;
    const float Icon_AlignY = 30;
    const Rectangle IconShape = {
        WINDOW_SCREEN_WIDTH/2 - IconWidth/2,
        LoginPanelShape.y + Icon_AlignY,
        IconWidth,
        IconHeight
    };
    DrawRectangleRounded(IconShape,0.1f,3, ColorAlpha(COLOR_DARKTHEME_PURPLE, 0.15f));

    DrawTexture(
        icon,
        WINDOW_SCREEN_WIDTH/2 - icon.width/2,
        LoginPanelShape.y + Icon_AlignY + (IconWidth - icon.height)/2,
        WHITE
    );


    // Draw Welcome Text
    const char* WelcomeText = "Welcome to Yahuu";
    Vector2 WelcomeTextPosition = {
        WINDOW_SCREEN_WIDTH/2 - MeasureTextEx(Font_Opensans_Bold_30,WelcomeText, 30, 2.5f).x/2,
        LoginPanelShape.y + Icon_AlignY + IconHeight + 20
    };
    DrawTextEx(
        Font_Opensans_Bold_30,
        WelcomeText,
        WelcomeTextPosition,
        30,
        2.5f,
        WHITE
    );


    //Draw Textfiled
    const int Spacing_TextFieldTitle = 1.0f;
    const Vector2 Position_UsernameTextField = {
        LoginPanelShape.x + 30,
        LoginPanelShape.y + Icon_AlignY + IconHeight + 100
    };
    DrawTextEx(
        Font_Opensans_Regular_20,
        "Username",
        Position_UsernameTextField,
        20,
        Spacing_TextFieldTitle,
        WHITE
    );

    Rectangle usernameTextFieldBounds = {
        LoginPanelShape.x + 30,
        LoginPanelShape.y + Icon_AlignY + IconHeight + 130,
        LoginPanelShape.width - 60,
        30
    };
    char usernamePlaceholder[] = "Enter your username";
   TextField usernameTextField = createTextField(usernamePlaceholder, dynamic_login_usernameValue, &dynamic_login_usernameIsActive, usernameTextFieldBounds, 0.2f, 10.0f, &Font_Opensans_Regular_20);
    drawTextField(&usernameTextField);

    const Vector2 Position_PasswordTextField = {
        LoginPanelShape.x + 30,
        LoginPanelShape.y + Icon_AlignY + IconHeight + 200
    };

    DrawTextEx(
        Font_Opensans_Regular_20,
        "Password",
        Position_PasswordTextField,
        20,
        Spacing_TextFieldTitle,
        WHITE
    );


    Rectangle passwordTextFieldBounds = {
        LoginPanelShape.x + 30,
        LoginPanelShape.y + Icon_AlignY + IconHeight + 230,
        LoginPanelShape.width - 60,
        30
    };
    char passwordPlaceholder[] = "Enter your password";
   TextField passwordTextField = createTextField(passwordPlaceholder, dynamic_login_passwordValue, &dynamic_login_passwordIsActive, passwordTextFieldBounds, 0.2f, 10.0f, &Font_Opensans_Regular_20);
    drawTextField(&passwordTextField);


    // ## Draw login button
    Rectangle loginButtonBounds = {
        LoginPanelShape.x + LoginPanelShape.width / 2 - 100,
        LoginPanelShape.y + LoginPanelShape.height - 110,
        200,
        40
    };

    RoundedButton loginButton = CreateRoundedButton(
        loginButtonBounds,
        COLOR_DARKTHEME_PURPLE,
        COLOR_DARKTHEME_BLACK,
        COLOR_DARKTHEME_BLACK,
        "Login",
        &Font_Opensans_Regular_20,
        20,
        login
    );
    DrawRoundedButton(loginButton);


    // ## Draw register button
    char registerButtonLabel[] = "Register";
    Vector2 registerButtonMeasure = MeasureTextEx(Font_Opensans_Bold_20, registerButtonLabel, 20, 0);
    Vector2 registerButtonPosition = {
        LoginPanelShape.x + LoginPanelShape.width / 2 - registerButtonMeasure.x / 2,
        LoginPanelShape.y + LoginPanelShape.height - 40
    };

    TextButton registerButton = CreateTextButton(
        registerButtonPosition,
        registerButtonLabel,
        Font_Opensans_Bold_20,
        20,
        COLOR_DARKTHEME_PURPLE,
        COLOR_DARKTHEME_BLACK,
        COLOR_DARKTHEME_BLACK,
        onClickRegisterButton
    );
    DrawTextButton(registerButton);
}
