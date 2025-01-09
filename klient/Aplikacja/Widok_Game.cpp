#include "Widok_Game.h"

Widok_Game::Widok_Game(sf::RenderWindow& window) : window(window) {
    // Room title
    roomTitle.setFont(Resources::getFont());
    roomTitle.setCharacterSize(24);
    roomTitle.setFillColor(sf::Color::Black);
    roomTitle.setPosition(300, 50);

    // Back button
    backButton.setSize(sf::Vector2f(100, 40));
    backButton.setFillColor(sf::Color(100, 149, 237));
    backButton.setPosition(50, 500);

    backButtonText.setFont(Resources::getFont());
    backButtonText.setString("Powrot");
    backButtonText.setCharacterSize(20);
    backButtonText.setFillColor(sf::Color::White);
    backButtonText.setPosition(65, 510);
}

bool Widok_Game::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
        if (backButton.getGlobalBounds().contains(mousePos)) {
            return true; // Signal to return to choice view
        }
    }
    return false;
}

void Widok_Game::render(const std::string& roomName) {
    roomTitle.setString("Pokoj: " + roomName);
    window.draw(roomTitle);
    window.draw(backButton);
    window.draw(backButtonText);
}
