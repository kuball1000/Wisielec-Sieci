#include "Resources.h"

sf::Font Resources::font;
std::vector<std::string> Resources::hangmanStages;

sf::Font& Resources::getFont() {
    if (font.getInfo().family.empty()) {
        font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/arial.ttf");
    }
    return font;
}

const std::vector<std::string>& Resources::getHangmanStages() {
    if (hangmanStages.empty()) {
        hangmanStages = {
    "   _____\n  |     |\n  |\n  |\n  |\n  |\n  |",            // 0: Początkowy stan
    "   _____\n  |     |\n  |     O\n  |\n  |\n  |\n  |",         // 1
    "   _____\n  |     |\n  |     O\n  |     |\n  |\n  |\n  |",   // 2
    "   _____\n  |     |\n  |     O\n  |    /|\n  |\n  |\n  |",   // 3
    "   _____\n  |     |\n  |     O\n  |    /|\\\n  |\n  |\n  |", // 4
    "   _____\n  |     |\n  |     O\n  |    /|\\\n  |    /\n  |\n  |",   // 5
    "   _____\n  |     |\n  |     O\n  |    /|\\\n  |    / \\\n  |\n  |"  // 6: Końcowy stan
        };
    }
    return hangmanStages;
}
