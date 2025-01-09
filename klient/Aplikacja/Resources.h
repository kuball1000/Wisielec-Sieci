#ifndef RESOURCES_H
#define RESOURCES_H

#include <SFML/Graphics.hpp>

class Resources {
public:
    static sf::Font& getFont();

private:
    static sf::Font font;
    static bool loaded;
};

#endif
