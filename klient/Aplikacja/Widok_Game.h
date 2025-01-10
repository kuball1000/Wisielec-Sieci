#ifndef WIDOK_GAME_H
#define WIDOK_GAME_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Widok_Game {
public:
    explicit Widok_Game(sf::RenderWindow& window);

    bool handleEvent(const sf::Event& event);
    void render(const std::string& roomName, const std::string& password, const std::vector<std::string>& playerNames, const std::vector<int>& playerStages);

private:
    sf::RenderWindow& window;
    sf::Text roomTitle;
    sf::RectangleShape backButton;
    sf::Text backButtonText;

    // Gracze
    sf::Text playerLabels[4];
    sf::Text hangmanStages[4];

    // Hasło
    sf::Text passwordTitleLabel; 
    sf::Text passwordLabel;

    // Input box
    sf::RectangleShape inputBox;
    sf::Text inputText;
    std::string enteredLetter;

    // Przycisk "Wyślij"
    sf::RectangleShape sendButton;
    sf::Text sendButtonText;

    // Liczba żyć
    sf::Text livesLabel;
    int lives;

    // Użyte litery
    sf::Text usedLettersLabel;
    std::string usedLetters;

    // Komunikaty serwera
    sf::Text serverMessagesLabel;
    std::string serverMessages;
};

#endif
