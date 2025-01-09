#include "Widok_Choice.h"
#include "Resources.h"

Widok_Choice::Widok_Choice(sf::RenderWindow& window) : window(window) {
    // Title
    title.setFont(Resources::getFont());
    title.setString("Wybierz opcje:");
    title.setCharacterSize(24);
    title.setPosition(300, 50);
    title.setFillColor(sf::Color::Black);

    // Create Room button
    createButton.setSize(sf::Vector2f(200, 40));
    createButton.setFillColor(sf::Color(100, 149, 237));
    createButton.setPosition(300, 150);

    createButtonText.setFont(Resources::getFont());
    createButtonText.setString("Stworz nowy pokoj");
    createButtonText.setCharacterSize(20);
    createButtonText.setFillColor(sf::Color::White);
    createButtonText.setPosition(310, 160);

    // Join Room button
    joinButton.setSize(sf::Vector2f(200, 40));
    joinButton.setFillColor(sf::Color(100, 149, 237));
    joinButton.setPosition(300, 220);

    joinButtonText.setFont(Resources::getFont());
    joinButtonText.setString("Dolacz do pokoju");
    joinButtonText.setCharacterSize(20);
    joinButtonText.setFillColor(sf::Color::White);
    joinButtonText.setPosition(310, 230);

    // Input box for room creation
    inputBox.setSize(sf::Vector2f(300, 40));
    inputBox.setFillColor(sf::Color(230, 230, 230));
    inputBox.setPosition(250, 150);

    inputLabel.setFont(Resources::getFont());
    inputLabel.setCharacterSize(20);
    inputLabel.setFillColor(sf::Color::Black);
    inputLabel.setPosition(260, 160);
}

bool Widok_Choice::handleEvent(const sf::Event& event, std::string& roomName) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
        if (createButton.getGlobalBounds().contains(mousePos)) {
            return true; // Switch to create room view
        } else if (joinButton.getGlobalBounds().contains(mousePos)) {
            return true; // Switch to join room view
        }
    }
    return false;
}

void Widok_Choice::render() {
    window.draw(title);
    window.draw(createButton);
    window.draw(createButtonText);
    window.draw(joinButton);
    window.draw(joinButtonText);
}

void Widok_Choice::renderCreateRoom() {
    window.draw(inputBox);
    window.draw(inputLabel);
}

void Widok_Choice::renderJoinRoom(const std::vector<std::string>& roomNames) {
    roomButtons.clear();
    roomButtonTexts.clear();

    for (size_t i = 0; i < roomNames.size(); ++i) {
        sf::RectangleShape button;
        button.setSize(sf::Vector2f(200, 40));
        button.setFillColor(sf::Color(100, 149, 237));
        button.setPosition(300, 150 + i * 60);

        sf::Text text;
        text.setFont(Resources::getFont());
        text.setString(roomNames[i]);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        text.setPosition(310, 160 + i * 60);

        roomButtons.push_back(button);
        roomButtonTexts.push_back(text);
    }

    for (size_t i = 0; i < roomButtons.size(); ++i) {
        window.draw(roomButtons[i]);
        window.draw(roomButtonTexts[i]);
    }
}
