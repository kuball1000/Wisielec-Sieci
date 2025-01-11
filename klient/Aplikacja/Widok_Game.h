#ifndef WIDOK_GAME_H
#define WIDOK_GAME_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Widok_Game {
public:
    explicit Widok_Game(sf::RenderWindow& window);

    bool handleLobbyEvent(const sf::Event& event); // Obsługa zdarzeń w widoku lobby
    bool handleGameEvent(const sf::Event& event); // Obsługa zdarzeń w widoku gry
    void renderLobby(const std::vector<std::string>& playerNames,bool flag); // Renderowanie widoku lobby
    void renderGame(const std::string& roomName, const std::string& password, const std::vector<std::string>& playerNames, const std::vector<int>& playerStages); // Renderowanie widoku gry

private:
    sf::RenderWindow& window;
    sf::Text roomTitle;
    sf::RectangleShape backButton;
    sf::Text backButtonText;

    // Lobby specific elements
    sf::RectangleShape startGameButton;
    sf::Text startGameButtonText;
    sf::Text lobbyPlayersText;

    // Gracze
    sf::Text playerLabels[4];
    sf::Text hangmanStages[4];

    // Hasło
    sf::Text passwordTitleLabel; // Dodano etykietę "Hasło:"
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

    bool backAction = false; // Flaga sygnalizująca powrót do wyboru pokoju
    bool startGameAction = false; // Flaga sygnalizująca rozpoczęcie gry
};

#endif
