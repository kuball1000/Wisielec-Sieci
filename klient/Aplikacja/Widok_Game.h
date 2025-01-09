#ifndef WIDOK_GAME_H
#define WIDOK_GAME_H

#include <SFML/Graphics.hpp>
#include "Resources.h"

class Widok_Game {
public:
    explicit Widok_Game(sf::RenderWindow& window);
    bool handleEvent(const sf::Event& event);
    void render(const std::string& roomName);

private:
    sf::RenderWindow& window;
    sf::Text roomTitle;
    sf::RectangleShape backButton;
    sf::Text backButtonText;
};

#endif
