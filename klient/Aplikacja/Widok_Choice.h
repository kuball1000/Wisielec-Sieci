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
    void resetToMainView();
    bool needsReset() const;

    void renderCreateRoom();
    void renderJoinRoom();
    void parseAvailableRooms(const std::string &serverResponse);

    // Dodajemy metodę do zarządzania listą pokoi
    void addRoom(const std::string& roomName);

    enum class State {
        MainView,
        CreateRoomView,
        JoinRoomView
    };

    State getCurrentState() const { return currentState; }

private:

    sf::RenderWindow& window;
    State currentState;

    // Główny widok
    sf::Text title;
    sf::RectangleShape createButton;
    sf::RectangleShape joinButton;
    sf::Text createButtonText;
    sf::Text joinButtonText;

    // Widok tworzenia pokoju
    sf::RectangleShape inputBox;
    sf::Text inputText;
    std::string roomNameInput;

    sf::RectangleShape confirmButton;
    sf::Text confirmButtonText;

    sf::RectangleShape backButton;
    sf::Text backButtonText;

    // Widok dołączania do pokoju
    std::vector<std::string> roomNames; // Lista nazw pokoi
    std::vector<sf::RectangleShape> roomButtons;
    std::vector<sf::Text> roomButtonTexts;
};

#endif
