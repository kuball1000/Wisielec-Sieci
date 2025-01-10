#include "Widok_Game.h"
#include "Resources.h"

Widok_Game::Widok_Game(sf::RenderWindow& window) : window(window), lives(5) {
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
    backButtonText.setString("Powrot");
    backButtonText.setCharacterSize(20);
    backButtonText.setFillColor(sf::Color::White);
    backButtonText.setPosition(65, 510);

    // Players and hangman stages
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

    // Password label
    passwordTitleLabel.setFont(Resources::getFont());
    passwordTitleLabel.setCharacterSize(24);
    passwordTitleLabel.setFillColor(sf::Color::Black);
    passwordTitleLabel.setString("Haslo:");
    passwordTitleLabel.setPosition(520, 400);

    // Password
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

bool Widok_Game::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
        if (backButton.getGlobalBounds().contains(mousePos)) {
            return true; // Signal to return to choice view
        } else if (sendButton.getGlobalBounds().contains(mousePos)) {
            // Logic for sending the letter
            if (!enteredLetter.empty()) {
                usedLetters += enteredLetter + " ";
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

void Widok_Game::render(const std::string& roomName, const std::string& password, const std::vector<std::string>& playerNames, const std::vector<int>& playerStages) {
    // Room title
    roomTitle.setString("Pokoj: " + roomName);
    window.draw(roomTitle);

    // Players and their stages
    const auto& hangmanStagesData = Resources::getHangmanStages();
    for (size_t i = 0; i < playerNames.size(); ++i) {
        playerLabels[i].setString(playerNames[i]);
        hangmanStages[i].setString(hangmanStagesData[playerStages[i]]);

        window.draw(playerLabels[i]);
        window.draw(hangmanStages[i]);
    }

    // Password title and password
    window.draw(passwordTitleLabel);
    passwordLabel.setString(password);
    window.draw(passwordLabel);

    // Input box
    window.draw(inputBox);
    inputText.setString(enteredLetter);
    window.draw(inputText);

    // Send button
    window.draw(sendButton);
    window.draw(sendButtonText);

    // Lives remaining
    livesLabel.setString("Zycia: " + std::to_string(lives));
    window.draw(livesLabel);

    // Used letters
    usedLettersLabel.setString("Uzyte litery: " + usedLetters);
    window.draw(usedLettersLabel);

    // Server messages
    serverMessagesLabel.setString("Wiadomosci serwera: " + serverMessages);
    window.draw(serverMessagesLabel);

    // Back button
    window.draw(backButton);
    window.draw(backButtonText);
}
