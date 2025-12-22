#include "components.h"
#include <raylib.h>
#include <stdio.h>

TextField createTextField(char *placeHolder, char *value, bool *isActive, Rectangle bounds, float roundedness, float segments, Font *font) {
    static int frameCounter = 0;
    if (*isActive) {
        frameCounter++;
        if (frameCounter > 10000) {
            frameCounter = 0;
        }
    }
    TextField textField = {
        .bounds = bounds,
        .roundedness = roundedness,
        .segments = segments,
        .font = font,
        .isActive = isActive,
        .framesCounter = frameCounter,
        .charCount = strlen(value),
        .text = value,
        .placeHolder = placeHolder,
    };
    return textField;
}

TextButton CreateTextButton(Vector2 pos, const char *text, Font font, float fontSize, Color colorNormal, Color colorHover, Color colorPressed, ButtonCallback callback) {
    TextButton btn = {
        .position = pos,
        .text = text,
        .font = font,
        .fontSize = fontSize,
        .spacing = 1.0f,
        .colorNormal = colorNormal,
        .colorHover = colorHover,
        .colorPressed = colorPressed,
        .callback = callback
    };
    return btn;
}

void drawTextField(TextField *textField)
{
    //Text Field logic
        if (CheckCollisionPointRec(GetMousePosition(), textField->bounds)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *textField->isActive = true;
            }
        } else {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *textField->isActive = false;
            }
        }

        if (*textField->isActive) {
            int key = GetCharPressed();

            if ((key >= 32) && (key <= 125) && (textField->charCount < MAX_TEXT_LENGTH)) {
                textField->text[textField->charCount] = (char)key;
                textField->charCount++;
                textField->text[textField->charCount] = '\0'; //End
            }

            if (IsKeyPressed(KEY_BACKSPACE) && (textField->charCount > 0)) {
                textField->text[--textField->charCount] = '\0';
            }

            textField->framesCounter++;
        }



        //Text Field GUI
        if (*textField->isActive) {
            DrawRectangleRec(textField->bounds, ColorAlpha(COLOR_DARKTHEME_BLACK, 0.2f));
            DrawRectangleRoundedLinesEx(textField->bounds, textField->roundedness, textField->segments, 1.0f, ColorAlpha(WHITE, 1.0f));
        } else {
            DrawRectangleRec(textField->bounds, ColorAlpha(COLOR_DARKTHEME_BLACK, 0.2f));
            DrawRectangleRoundedLinesEx(textField->bounds, textField->roundedness, textField->segments, 1.0f, ColorAlpha(WHITE, 0.4f));
        }

        Vector2 textPos = {
            (int)textField->bounds.x + 5,
            (int)textField->bounds.y + 8
        };
        DrawTextEx(
            *textField->font,
            textField->text,
            textPos,
            20,
            1.0f,
            WHITE
        );


        // Draw cursor
        if (*textField->isActive) {
            if ((textField->framesCounter/20) % 2 == 0) {
                Vector2 textMesuare = MeasureTextEx(*textField->font, textField->text, 20, 1.0f);
                int textWidth = textMesuare.x;
                Vector2 startPos = {
                    (int)textField->bounds.x + textWidth + 5,
                    (int)textField->bounds.y + 4
                };
                Vector2 endPos = {
                    (int)textField->bounds.x + textWidth + 5,
                    (int)textField->bounds.y + textField->bounds.height - 4
                };
                DrawLineEx(startPos, endPos, 2.0f, WHITE);
            }
        } else {
            if (textField->charCount == 0) {
                Vector2 placeholderPos = {
                    (int)textField->bounds.x + 5,
                    (int)textField->bounds.y + 8
                };
                DrawTextEx(
                    *textField->font,
                    textField->placeHolder,
                    placeholderPos,
                    20,
                    1.0f,
                    WHITE
                );
            }
        }
}






void DrawTextButton(TextButton btn) {
    // 1. Tính kích thước thực tế của text để làm vùng va chạm (Hitbox)
    Vector2 textSize = MeasureTextEx(btn.font, btn.text, btn.fontSize, btn.spacing);

    Rectangle bounds = {
        btn.position.x,
        btn.position.y,
        textSize.x,
        textSize.y
    };

    Vector2 mousePoint = GetMousePosition();
    bool isHover = CheckCollisionPointRec(mousePoint, bounds);
    bool isClicked = false;

    // 2. Xử lý Logic màu sắc
    Color drawColor = btn.colorNormal;

    if (isHover) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            drawColor = btn.colorPressed; // Đang nhấn giữ
        } else {
            drawColor = btn.colorHover;   // Chỉ trỏ chuột vào
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            btn.callback();
        }
    }

    // 3. Vẽ Text
    DrawTextEx(btn.font, btn.text, btn.position, btn.fontSize, btn.spacing, drawColor);

}

RoundedButton CreateRoundedButton(Rectangle rect, Color bgColor, Color hoverColor, Color pressedColor, const char *text, Font *font, float fontSize, ButtonCallback callback) {
    RoundedButton btn = {
        .bounds = rect,
        .text = text,
        .font = font,
        .fontSize = fontSize,
        // Màu tím giống trong hình (R:160, G:32, B:240 là tím chuẩn, ta chỉnh cho giống ảnh hơn)
        .bgColor = bgColor,
        .textColor = WHITE,
        .hoverColor = hoverColor,
        .pressedColor = pressedColor,
        .roundness = 0.5f, // 0.5 là bo tròn khá nhiều, giống hình viên thuốc
        .onClick = callback
    };
    return btn;
}

void DrawRoundedButton(RoundedButton btn) {
    Vector2 mousePoint = GetMousePosition();
    bool isHover = CheckCollisionPointRec(mousePoint, btn.bounds);
    bool isPressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool isClicked = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    // 1. Xử lý màu sắc khi tương tác (Hover/Click)
    Color drawColor = btn.bgColor;

    if (isHover) {
        if (isPressed) {
            // Khi nhấn: Màu tối đi một chút
            drawColor = btn.pressedColor;
        } else {
            // Khi hover: Màu sáng lên một chút (hiệu ứng highlight)
            drawColor = btn.hoverColor;
        }
    }

    // 2. Vẽ nền bo tròn (Quan trọng: segments = 10 để góc bo mượt)
    DrawRectangleRounded(btn.bounds, btn.roundness, 10, drawColor);

    // 3. Tính toán để CĂN GIỮA chữ trong nút
    Vector2 textSize = MeasureTextEx(*btn.font, btn.text, btn.fontSize, 1.0f);

    Vector2 textPos;
    textPos.x = btn.bounds.x + (btn.bounds.width - textSize.x) / 2; // Căn giữa ngang
    textPos.y = btn.bounds.y + (btn.bounds.height - textSize.y) / 2; // Căn giữa dọc

    // 4. Vẽ chữ
    DrawTextEx(*btn.font, btn.text, textPos, btn.fontSize, 1.0f, btn.textColor);

    // 5. Gọi hàm callback nếu click xong
    if (isHover && isClicked && btn.onClick != NULL) {
        btn.onClick();
    }
}

void ChatListButton(Rectangle bounds,
                    const char* text,
                    Font font, float fontSize,
                    Color bgNormal, Color bgHover, Color bgPress,
                    Color textColor,
                    ButtonCallback callback)
{
    Vector2 mouse = GetMousePosition();
    bool isHover = CheckCollisionPointRec(mouse, bounds);
    bool isDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    bool isClick = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    // 1. Xử lý màu nền
    Color currentBg = bgNormal;
    if (isHover) currentBg = isDown ? bgPress : bgHover;

    DrawRectangleRec(bounds, currentBg);

    // 2. Tính toán vị trí text (Căn giữa theo chiều dọc, lùi vào trái 15px)
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 1.0f);
    Vector2 textPos = {
        bounds.x + 15.0f,
        bounds.y + (bounds.height - textSize.y) / 2.0f
    };

    DrawTextEx(font, text, textPos, fontSize, 1.0f, textColor);

    // 3. Callback
    if (isHover && isClick && callback) callback();
}



void DrawBubbleChat(Rectangle bounds,
                    const char* text,
                    Font font,
                    float fontSize,
                    Color textColor,
                    bool isMe) {
    Vector2 measureTextEx = MeasureTextEx(font, text, fontSize, 1.0f);
    printf("%f", measureTextEx.x);
    printf("%f", bounds.x);
    if (isMe) {
        Rectangle bubble = {
            200 + bounds.width - measureTextEx.x - 20,
            bounds.y,
            // measureTextEx.x + 20,
            // measureTextEx.y + 20
            bounds.width,
            bounds.height
        };
        DrawRectangleRounded(bubble,0.3f, 10, ColorAlpha(WHITE, 0.5f));
        DrawTextEx(font, text, (Vector2){bubble.x + 10, bubble.y + 10}, fontSize, 1.0f, textColor);
    } else {
        Rectangle bubble = {
            200 + 20,
            bounds.y,
            measureTextEx.x + 20,
            measureTextEx.y + 20
        };
        DrawRectangleRounded(bubble, 0.3f, 10, ColorAlpha(WHITE, 0.5f));
        DrawTextEx(font, text, (Vector2){bubble.x + 10, bubble.y + 10}, fontSize, 1.0f, textColor);
    }
}
