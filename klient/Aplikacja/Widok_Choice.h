#ifndef WIDOK_CHOICE_H
#define WIDOK_CHOICE_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Widok_Choice {
public:
    explicit Widok_Choice(sf::RenderWindow& window);
    bool handleEvent(const sf::Event& event, std::string& roomName);
    void render();
    void renderCreateRoom();
    void renderJoinRoom(const std::vector<std::string>& roomNames);

private:
    sf::RenderWindow& window;
    sf::Text title;
    sf::RectangleShape createButton;
    sf::RectangleShape joinButton;
    sf::Text createButtonText;
    sf::Text joinButtonText;

    sf::RectangleShape inputBox;
    sf::Text inputLabel;
    std::string inputText;

    std::vector<sf::RectangleShape> roomButtons;
    std::vector<sf::Text> roomButtonTexts;
};

#endif
