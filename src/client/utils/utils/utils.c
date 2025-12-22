#include "utils.h"

Texture2D loadResizeImage(const char* path, int width, int height) {
    Image image = LoadImage(path);
    ImageResize(&image, width, height);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}
