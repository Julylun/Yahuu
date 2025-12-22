#include "registerScreen.h"
#include <raylib.h>

void drawRegisterScreen()
{
    ClearBackground(COLOR_DARKTHEME_BLACK);

       //## Draw Panel
       const float LoginPanelWidth = 400.0f;
       const float LoginPanelHeight = 500.0f;
       const Rectangle RegisterPanelShape = {
           WINDOW_SCREEN_WIDTH / 2 - LoginPanelWidth/2,
           WINDOW_SCREEN_HEIGHT / 2 - LoginPanelHeight/2,
           LoginPanelWidth,
           LoginPanelHeight
       };

       DrawRectangleRounded(RegisterPanelShape,0.1f,3, ColorAlpha(COLOR_DARKTHEME_GRAY, 255.f));

       //## Draw Icon
       const float IconWidth = 60;
       const float IconHeight = 60;
       const float Icon_AlignY = 30;
       const Rectangle IconShape = {
           WINDOW_SCREEN_WIDTH/2 - IconWidth/2,
           RegisterPanelShape.y + Icon_AlignY,
           IconWidth,
           IconHeight
       };
       DrawRectangleRounded(IconShape,0.1f,3, ColorAlpha(COLOR_DARKTHEME_PURPLE, 0.15f));

       Texture2D icon = loadResizeImage("assets/icon.png", 80, 80);
       DrawTexture(
           icon,
           WINDOW_SCREEN_WIDTH/2 - icon.width/2,
           RegisterPanelShape.y + Icon_AlignY + (IconWidth - icon.height)/2,
           WHITE
       );


       // Draw Welcome Text
       const char* Text_RegisterTitle = "Join our party?";
       const int Font_Size_RegisterTitle = 30;
       const Font Font_OpenSans_Bold = LoadFontEx("assets/fonts/opensans/OpenSans-SemiBold.ttf", Font_Size_RegisterTitle, 0, 0);
       Vector2 Position_WelcomeText = {
           WINDOW_SCREEN_WIDTH/2 - MeasureTextEx(Font_OpenSans_Bold,Text_RegisterTitle, Font_Size_RegisterTitle, 2.5f).x/2,
           RegisterPanelShape.y + Icon_AlignY + IconHeight + 20
       };
       DrawTextEx(
           Font_OpenSans_Bold,
           Text_RegisterTitle,
           Position_WelcomeText,
           Font_Size_RegisterTitle,
           2.5f,
           WHITE
       );


       //Draw Textfiled
       const int Spacing_TextFieldTitle = 1.0f;
       Font Font_OpenSans_Regular = LoadFontEx("assets/fonts/opensans/OpenSans-Regular.ttf", 20, 0, 0);
       const Vector2 Position_UsernameTextField = {
           RegisterPanelShape.x + 30,
           RegisterPanelShape.y + Icon_AlignY + IconHeight + 100
       };
       DrawTextEx(
           Font_OpenSans_Regular,
           "Username",
           Position_UsernameTextField,
           20,
           Spacing_TextFieldTitle,
           WHITE
       );

       Rectangle usernameTextFieldBounds = {
           RegisterPanelShape.x + 30,
           RegisterPanelShape.y + Icon_AlignY + IconHeight + 130,
           RegisterPanelShape.width - 60,
           30
       };
       char usernamePlaceholder[] = "Enter your username";
       static char dynamic_usernameValue[100] = "";
       static bool dynamic_usernameIsActive = false;
       TextField usernameTextField = createTextField(usernamePlaceholder, dynamic_usernameValue, &dynamic_usernameIsActive, usernameTextFieldBounds, 0.2f, 10.0f, &Font_OpenSans_Regular);
       drawTextField(&usernameTextField);

       const Vector2 Position_PasswordTextField = {
           RegisterPanelShape.x + 30,
           RegisterPanelShape.y + Icon_AlignY + IconHeight + 180
       };

       DrawTextEx(
           Font_OpenSans_Regular,
           "Password",
           Position_PasswordTextField,
           20,
           Spacing_TextFieldTitle,
           WHITE
       );

       Rectangle passwordTextFieldBounds = {
           RegisterPanelShape.x + 30,
           RegisterPanelShape.y + Icon_AlignY + IconHeight + 210,
           RegisterPanelShape.width - 60,
           30
       };
       char passwordPlaceholder[] = "Enter your password";
       static char dynamic_passwordValue[100] = "";
       static bool dynamic_passwordIsActive = false;
       TextField passwordTextField = createTextField(passwordPlaceholder, dynamic_passwordValue, &dynamic_passwordIsActive, passwordTextFieldBounds, 0.2f, 10.0f, &Font_OpenSans_Regular);
       drawTextField(&passwordTextField);





       const Vector2 Position_RepeatPasswordTextField = {
           RegisterPanelShape.x + 30,
           RegisterPanelShape.y + Icon_AlignY + IconHeight + 260
       };

       DrawTextEx(
           Font_OpenSans_Regular,
           "Repeat Password",
           Position_RepeatPasswordTextField,
           20,
           Spacing_TextFieldTitle,
           WHITE
       );

       Rectangle repeatPasswordTextFieldBounds = {
           RegisterPanelShape.x + 30,
           RegisterPanelShape.y + Icon_AlignY + IconHeight + 280,
           RegisterPanelShape.width - 60,
           30
       };
       char repeatPasswordPlaceholder[] = "Repeat Password";
       static char dynamic_repeatPasswordValue[100] = "";
       static bool dynamic_repeatPasswordIsActive = false;
       TextField repeatPasswordTextField = createTextField(repeatPasswordPlaceholder, dynamic_repeatPasswordValue, &dynamic_repeatPasswordIsActive, repeatPasswordTextFieldBounds, 0.2f, 10.0f, &Font_OpenSans_Regular);
       drawTextField(&passwordTextField);
       drawTextField(&repeatPasswordTextField);



       // ## Draw login button
       Rectangle Position_ReigsterButton = {
           RegisterPanelShape.x + RegisterPanelShape.width / 2 - 100,
           RegisterPanelShape.y + RegisterPanelShape.height - 75,
           200,
           40
       };

       RoundedButton Button_Register = CreateRoundedButton(
           Position_ReigsterButton,
           COLOR_DARKTHEME_PURPLE,
           COLOR_DARKTHEME_BLACK,
           COLOR_DARKTHEME_BLACK,
           "Register",
           &Font_OpenSans_Regular,
           20,
           Register_onClickRegisterButton
       );
       DrawRoundedButton(Button_Register);


       // ## Draw register button
       char Text_LoginButton[] = "I have an account";
       Vector2 Measure_LoginButton = MeasureTextEx(Font_OpenSans_Bold, Text_LoginButton, 20, 1.0f);
       Vector2 Position_LoginButton = {
           RegisterPanelShape.x + RegisterPanelShape.width / 2 - Measure_LoginButton.x / 2,
           RegisterPanelShape.y + RegisterPanelShape.height - 30
       };

       TextButton Button_Login = CreateTextButton(
           Position_LoginButton,
           Text_LoginButton,
           Font_OpenSans_Bold,
           20,
           COLOR_DARKTHEME_PURPLE,
           COLOR_DARKTHEME_BLACK,
           COLOR_DARKTHEME_BLACK,
           Register_onClickIhaveAccount
       );
       DrawTextButton(Button_Login);

}
