#include "Widok_Game.h"
#include "Resources.h"
#include <iostream>
#include "Application.h" 

Widok_Game::Widok_Game(sf::RenderWindow& window, Application* app)
    : window(window), application(app), lives(5), backAction(false), startGameAction(false) {
            if (!application) {
        std::cerr << "Błąd: wskaźnik application nie został poprawnie przekazany do Widok_Game." << std::endl;
    }
    // Room title
    roomTitle.setFont(Resources::getFont());
    roomTitle.setCharacterSize(24);
    roomTitle.setFillColor(sf::Color::Black);
    roomTitle.setPosition(300, 20);

    // Back button
    backButton.setSize(sf::Vector2f(100, 40));
    backButton.setFillColor(sf::Color(100, 149, 237));
    backButton.setPosition(50, 500);

    backButtonText.setFont(Resources::getFont());
    backButtonText.setString("Wyjscie");
    backButtonText.setCharacterSize(20);
    backButtonText.setFillColor(sf::Color::White);
    backButtonText.setPosition(65, 510);

    // Start game button
    startGameButton.setSize(sf::Vector2f(150, 40));
    startGameButton.setFillColor(sf::Color(0, 128, 0));
    startGameButton.setPosition(600, 500);

    startGameButtonText.setFont(Resources::getFont());
    startGameButtonText.setString("Rozpocznij gre");
    startGameButtonText.setCharacterSize(20);
    startGameButtonText.setFillColor(sf::Color::White);
    startGameButtonText.setPosition(610, 510);

    // Lobby players text
    lobbyPlayersText.setFont(Resources::getFont());
    lobbyPlayersText.setCharacterSize(20);
    lobbyPlayersText.setFillColor(sf::Color::Black);
    lobbyPlayersText.setPosition(300, 200);

    // Game-related UI elements (hidden in lobby)
    for (int i = 0; i < 4; ++i) {
        playerLabels[i].setFont(Resources::getFont());
        playerLabels[i].setCharacterSize(18);
        playerLabels[i].setFillColor(sf::Color::Black);
        playerLabels[i].setPosition(50 + i * 200, 100);

        hangmanStages[i].setFont(Resources::getFont());
        hangmanStages[i].setCharacterSize(18);
        hangmanStages[i].setFillColor(sf::Color::Black);
        hangmanStages[i].setPosition(50 + i * 200, 150);
    }

    passwordTitleLabel.setFont(Resources::getFont());
    passwordTitleLabel.setCharacterSize(24);
    passwordTitleLabel.setFillColor(sf::Color::Black);
    passwordTitleLabel.setString("Haslo:");
    passwordTitleLabel.setPosition(520, 400);

    passwordLabel.setFont(Resources::getFont());
    passwordLabel.setCharacterSize(24);
    passwordLabel.setFillColor(sf::Color::Black);
    passwordLabel.setPosition(600, 400);

    // Input box
    inputBox.setSize(sf::Vector2f(100, 40));
    inputBox.setFillColor(sf::Color(230, 230, 230));
    inputBox.setPosition(50, 350);

    inputText.setFont(Resources::getFont());
    inputText.setCharacterSize(20);
    inputText.setFillColor(sf::Color::Black);
    inputText.setPosition(60, 360);

    // Send button
    sendButton.setSize(sf::Vector2f(100, 40));
    sendButton.setFillColor(sf::Color(100, 149, 237));
    sendButton.setPosition(160, 350);

    sendButtonText.setFont(Resources::getFont());
    sendButtonText.setString("Wyslij");
    sendButtonText.setCharacterSize(20);
    sendButtonText.setFillColor(sf::Color::White);
    sendButtonText.setPosition(175, 360);

    // Lives remaining
    livesLabel.setFont(Resources::getFont());
    livesLabel.setCharacterSize(24);
    livesLabel.setFillColor(sf::Color::Black);
    livesLabel.setPosition(650, 500);

    // Used letters
    usedLettersLabel.setFont(Resources::getFont());
    usedLettersLabel.setCharacterSize(18);
    usedLettersLabel.setFillColor(sf::Color::Black);
    usedLettersLabel.setPosition(50, 400);

    // Server messages
    serverMessagesLabel.setFont(Resources::getFont());
    serverMessagesLabel.setCharacterSize(18);
    serverMessagesLabel.setFillColor(sf::Color::Black);
    serverMessagesLabel.setPosition(50, 450);

}

bool Widok_Game::handleLobbyEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

        if (backButton.getGlobalBounds().contains(mousePos)) {
            backAction = true; // Ustawienie flagi powrotu
            std::cout << "Wysyłanie komendy /exit do serwera." << std::endl;
            if (!application->sendMessage("/exit")) {
                   std::cout << "Błąd wysyłania komendy do serwera: /exit" << std::endl;
            } else {
                window.close();
            }
        }

        if (startGameButton.getGlobalBounds().contains(mousePos)) {
            startGameAction = true; // Ustawienie flagi rozpoczęcia gry
            return true;            // Przejście do widoku gry
        }
    }

    return false;
}

bool Widok_Game::handleGameEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

        // Obsługa przycisku "Wyjscie"
        if (backButton.getGlobalBounds().contains(mousePos)) {
            if (!application->sendMessage("/exit")) {
                //    std::cer << "Błąd wysyłania komendy do serwera: /exit" << std::endl;
            } else {
                window.close();
            }
        }

        // Obsługa przycisku "Wyślij"
        if (sendButton.getGlobalBounds().contains(mousePos)) {
            if (!enteredLetter.empty()) {
                std::cout << enteredLetter << std::endl;
                // Dodaj literę do użytych liter

                    if (application && !application->sendMessage(enteredLetter)) {
                        serverMessages = "Błąd wysyłania litery do serwera.";
                    } else if (!application) {
                        serverMessages = "Błąd: wskaźnik application jest pusty.";
                    }
                    
                    usedLetters += enteredLetter + " ";

                // Wyczyść pole tekstowe
                enteredLetter.clear();
            }
        }
    } else if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == 8 && !enteredLetter.empty()) { // Backspace
            enteredLetter.pop_back();
        } else if (event.text.unicode >= 'a' && event.text.unicode <= 'z') {
            enteredLetter = static_cast<char>(event.text.unicode);
        }
    }

    return false;
}



void Widok_Game::renderLobby(const std::vector<std::string>& playerNames,bool flag,const std::string& roomName) {
    // Room title
    roomTitle.setString("Lobby: " +roomName); 
    window.draw(roomTitle);

    // Display players in the lobby
    std::string playersList = "Gracze w pokoju:\n";
    for (const auto& player : playerNames) {
        playersList += player + "\n";
    }
    lobbyPlayersText.setString(playersList);
    window.draw(lobbyPlayersText);

    // Render buttons
    window.draw(backButton); // Usunięcie przycisku powrotu do mozliwe pozniejszej implementacji
    window.draw(backButtonText);


    if (flag && playerNames.size() > 1) {
        window.draw(startGameButton);
        window.draw(startGameButtonText);
    }

}

void Widok_Game::renderGame(const std::string& roomName, const std::string& password,
                            const std::string& usedLetters, int lives,
                            const std::vector<std::string>& playerNames,
                            const std::vector<int>& playerStages, const std::string& newServerMessage) {
    // Room title
    roomTitle.setString("Pokoj: " + roomName);
    window.draw(roomTitle);

    const auto& hangmanStagesData = Resources::getHangmanStages();
    for (size_t i = 0; i < playerNames.size(); ++i) {
        playerLabels[i].setString(playerNames[i]);
        hangmanStages[i].setString(hangmanStagesData[playerStages[i]]);

        window.draw(playerLabels[i]);
        window.draw(hangmanStages[i]);
    }

    // Render password
    window.draw(passwordTitleLabel);
    passwordLabel.setString(password);
    window.draw(passwordLabel);

     // Render input box
    window.draw(inputBox);
    inputText.setString(enteredLetter);
    window.draw(inputText);

    // Render send button
    window.draw(sendButton);
    window.draw(sendButtonText);

    // Render lives remaining
    livesLabel.setString("Zycia: " + std::to_string(lives));
    window.draw(livesLabel);

    // Render used letters
    usedLettersLabel.setString("Uzyte litery: " + usedLetters);
    window.draw(usedLettersLabel);

    //Render server messages
      if (!newServerMessage.empty() && newServerMessage != serverMessages) {
        serverMessages = newServerMessage;
        serverMessageTime = std::chrono::steady_clock::now();
    }

    // Check if the message should still be rendered
    auto now = std::chrono::steady_clock::now();
    if (now - serverMessageTime < std::chrono::seconds(5)) {
        serverMessagesLabel.setString("Wiadomosci serwera: " + serverMessages);
        window.draw(serverMessagesLabel);
    }else{
        serverMessagesLabel.setString("Wiadomosci serwera: ");
        window.draw(serverMessagesLabel);
    }

    // Render back button
    window.draw(backButton); // Usunięcie przycisku powrotu do mozliwe pozniejszej implementacji
    window.draw(backButtonText);
}
