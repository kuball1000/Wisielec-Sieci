#ifndef WIDOK_LOGIN_H
#define WIDOK_LOGIN_H

#include <SFML/Graphics.hpp>
#include "Resources.h"

class Widok_Login {
public:
    explicit Widok_Login(sf::RenderWindow& window);
    bool handleEvent(const sf::Event& event);
    void render();

    const std::string& getNick() const { return inputText; }
private:
    sf::RenderWindow& window;
    sf::Text title;
    sf::Text inputLabel;
    sf::RectangleShape inputBox;
    sf::RectangleShape button;
    sf::Text buttonLabel;

    std::string inputText;
    bool buttonPressed;
};

#endif
