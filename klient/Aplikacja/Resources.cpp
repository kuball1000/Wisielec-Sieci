#include "Resources.h"

sf::Font Resources::font;
bool Resources::loaded = false;

sf::Font& Resources::getFont() {
    if (!loaded) {
        font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/arial.ttf");
        loaded = true;
    }
    return font;
}
