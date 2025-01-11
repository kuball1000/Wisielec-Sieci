#include "Widok_Login.h"

Widok_Login::Widok_Login(sf::RenderWindow& window) : window(window), buttonPressed(false) {
    // Title
    title.setFont(Resources::getFont());
    title.setString("Podaj swoj nick:");
    title.setCharacterSize(24);
    title.setPosition(300, 100);
    title.setFillColor(sf::Color::Black);

    // Input box
    inputBox.setSize(sf::Vector2f(200, 40));
    inputBox.setFillColor(sf::Color(230, 230, 230));
    inputBox.setPosition(300, 200);

    // Button
    button.setSize(sf::Vector2f(100, 40));
    button.setFillColor(sf::Color(100, 149, 237));
    button.setPosition(350, 300);

    buttonLabel.setFont(Resources::getFont());
    buttonLabel.setString("OK");
    buttonLabel.setCharacterSize(20);
    buttonLabel.setFillColor(sf::Color::White);
    buttonLabel.setPosition(375, 310);

    // Error message
    errorMessage.setFont(Resources::getFont());
    errorMessage.setCharacterSize(18);
    errorMessage.setFillColor(sf::Color::Red);
    errorMessage.setPosition(280, 400);
    errorMessage.setString(""); // Początkowo pusty
}

bool Widok_Login::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == 8 && !inputText.empty()) {
            inputText.pop_back();
        } else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
            inputText += static_cast<char>(event.text.unicode);
        }
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            if (button.getGlobalBounds().contains(mousePos)) {
                buttonPressed = true;
                return true; // Signal to change view
            }
        }
    }
    return false;
}

void Widok_Login::render() {
    sf::Text inputTextDisplay;
    inputTextDisplay.setFont(Resources::getFont());
    inputTextDisplay.setString(inputText);
    inputTextDisplay.setCharacterSize(20);
    inputTextDisplay.setPosition(310, 210);
    inputTextDisplay.setFillColor(sf::Color::Black);

    window.draw(title);
    window.draw(inputBox);
    window.draw(inputTextDisplay);
    window.draw(button);
    window.draw(buttonLabel);

     if (!errorMessage.getString().isEmpty()) {
        auto now = std::chrono::steady_clock::now();
        if (now - errorMessageTimer < std::chrono::seconds(3)) {
            window.draw(errorMessage);
        } else {
            errorMessage.setString(""); // Ukrycie komunikatu po 3 sekundach
        }
    }
}
void Widok_Login::showErrorMessage(const std::string& message) {
    errorMessage.setString(message);
    errorMessageTimer = std::chrono::steady_clock::now();
}
