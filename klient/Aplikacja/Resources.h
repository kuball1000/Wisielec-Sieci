#ifndef RESOURCES_H
#define RESOURCES_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Resources {
public:
    static sf::Font& getFont();
    static const std::vector<std::string>& getHangmanStages();

private:
    static sf::Font font;
    static std::vector<std::string> hangmanStages;
};

#endif
