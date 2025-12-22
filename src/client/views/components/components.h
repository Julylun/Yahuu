#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <string.h>
#include <raylib.h>
#include "../../utils/colors/colors.h"

#define MAX_TEXT_LENGTH 99999

// Định nghĩa một kiểu dữ liệu mới tên là "ButtonCallback"
// Nó đại diện cho một hàm có dạng: void TenHam(void)
typedef void (*ButtonCallback)(void);
typedef struct {
    int limitInput;
    char *text;
    int charCount;

    Rectangle bounds;
    bool *isActive;
    int framesCounter;
    float roundedness;
    float segments;
    Font *font;
    char *placeHolder;
} TextField;
void drawTextField(TextField* textField);
TextField createTextField(char *placeHolder, char *value, bool *isActive, Rectangle bounds, float roundedness, float segments, Font *font);


typedef struct TextButton {
    Vector2 position;       // Vị trí vẽ
    const char *text;       // Nội dung chữ
    Font font;              // Font chữ
    float fontSize;         // Kích thước
    float spacing;          // Khoảng cách chữ
    Color colorNormal;      // Màu bình thường
    Color colorHover;       // Màu khi di chuột vào
    Color colorPressed;     // Màu khi đang nhấn giữ

    ButtonCallback callback; // Hàm callback khi nhấn nút
} TextButton;

typedef struct RoundedButton {
    Rectangle bounds;       // Vị trí và kích thước
    const char *text;       // Nội dung chữ ("Log In")
    Font *font;             // Font chữ
    float fontSize;         // Cỡ chữ
    Color bgColor;          // Màu nền (Tím)
    Color textColor;        // Màu chữ (Trắng)
    Color hoverColor;       // Màu khi di chuột vào
    Color pressedColor;     // Màu khi đang nhấn giữ
    float roundness;        // Độ bo tròn (0.0 đến 1.0)
    ButtonCallback onClick; // Hàm xử lý khi click
} RoundedButton;

void DrawTextButton(TextButton btn);
TextButton CreateTextButton(Vector2 pos, const char *text, Font font, float fontSize, Color colorNormal, Color colorHover, Color colorPressed, ButtonCallback callback);

RoundedButton CreateRoundedButton(Rectangle rect, Color bgColor, Color hoverColor, Color pressedColor, const char *text, Font *font, float fontSize, ButtonCallback callback);
void DrawRoundedButton(RoundedButton btn);
#endif
