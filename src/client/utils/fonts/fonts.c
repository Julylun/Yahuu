#include "fonts.h"

Font Font_Opensans_Bold_30 = {0};
Font Font_Opensans_Bold_20 = {0};
Font Font_Opensans_Regular_20 = {0};
Font Font_Opensans_Bold_17 = {0};


void debugFont(Font font, const char* fontName)
{
    if (font.baseSize == 0)
    {
        setDebugMessage(TextFormat("Failed to load font: %s", fontName));
    }
    else
    {
        setDebugMessage(TextFormat("Loaded font: %s", fontName));
    }
}
void loadFonts()
{

    // Load fonts here
    Font_Opensans_Bold_30 = LoadFontEx(
        "assets/fonts/opensans/OpenSans-SemiBold.ttf",
        30,
        0,
        0
    );

    Font_Opensans_Regular_20 = LoadFontEx(
        "assets/fonts/opensans/OpenSans-Regular.ttf",
        20,
        0,
        0
    );

    Font_Opensans_Bold_20 = LoadFontEx(
        "assets/fonts/opensans/OpenSans-SemiBold.ttf",
        20,
        0,
        0
    );
    Font_Opensans_Bold_17 = LoadFontEx(
        "assets/fonts/opensans/OpenSans-SemiBold.ttf",
        17,
        0,
        0
    );

    debugFont(Font_Opensans_Bold_30, "OpenSans-SemiBold.ttf - size 30");
    debugFont(Font_Opensans_Bold_20, "OpenSans-SemiBold.ttf - size 20");
    debugFont(Font_Opensans_Regular_20, "OpenSans-Regular.ttf - size 20");
    debugFont(Font_Opensans_Bold_17, "OpenSans-SemiBold.ttf - size 17");
}
