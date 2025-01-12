#include "Widok_Login.h"
#include <iostream>

Widok_Login::Widok_Login(sf::RenderWindow& window) 
    : window(window), buttonPressed(false), activeInput(InputType::Nick), ipText("127.0.0.1") {
    // Tytuł
    title.setFont(Resources::getFont());
    title.setString("Podaj dane logowania:");
    title.setCharacterSize(24);
    title.setFillColor(sf::Color::Black);
    title.setPosition(300, 50);

    // Pole nicku
    inputBoxNick.setSize(sf::Vector2f(200, 40));
    inputBoxNick.setFillColor(sf::Color(230, 230, 230));
    inputBoxNick.setPosition(300, 400);

    // Label IP
    inputLabelIP.setFont(Resources::getFont());
    inputLabelIP.setString("Adres IP:");
    inputLabelIP.setCharacterSize(20);
    inputLabelIP.setFillColor(sf::Color::Black);
    inputLabelIP.setPosition(200, 120);

    // Pole IP
    inputBoxIP.setSize(sf::Vector2f(200, 40));
    inputBoxIP.setFillColor(sf::Color(230, 230, 230));
    inputBoxIP.setPosition(300, 120);

    // Przycisk
    button.setSize(sf::Vector2f(100, 40));
    button.setFillColor(sf::Color(100, 149, 237));
    button.setPosition(550, 400);

    buttonLabel.setFont(Resources::getFont());
    buttonLabel.setString("Zaloguj");
    buttonLabel.setCharacterSize(20);
    buttonLabel.setFillColor(sf::Color::White);
    buttonLabel.setPosition(
        button.getPosition().x + (button.getSize().x - buttonLabel.getGlobalBounds().width) / 2,
        button.getPosition().y + (button.getSize().y - buttonLabel.getGlobalBounds().height) / 2
    );

    // Komunikat o błędzie
    errorMessage.setFont(Resources::getFont());
    errorMessage.setCharacterSize(18);
    errorMessage.setFillColor(sf::Color::Red);
    errorMessage.setPosition(300, 500);
    errorMessage.setString(""); // Początkowo pusty

    ipButton.setSize(sf::Vector2f(150, 40));
    ipButton.setFillColor(sf::Color(100, 149, 237));
    ipButton.setPosition(550, 120);

    ipButtonLabel.setFont(Resources::getFont());
    ipButtonLabel.setString("Wyslij IP");
    ipButtonLabel.setCharacterSize(20);
    ipButtonLabel.setFillColor(sf::Color::White);
    ipButtonLabel.setPosition(
        ipButton.getPosition().x + (ipButton.getSize().x - ipButtonLabel.getGlobalBounds().width) / 2,
        ipButton.getPosition().y + (ipButton.getSize().y - ipButtonLabel.getGlobalBounds().height) / 2
    );
}

bool Widok_Login::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

        // Sprawdź, który input box został kliknięty
        if (inputBoxNick.getGlobalBounds().contains(mousePos)) {
            activeInput = InputType::Nick;
        } else if (inputBoxIP.getGlobalBounds().contains(mousePos)) {
            activeInput = InputType::IP;
        } else if (button.getGlobalBounds().contains(mousePos)) {
            std::cout << "Przycisk Zaloguj kliknięty!" << std::endl; // Debug
            buttonPressed = true;
            return true; // Sygnalizowanie, że przycisk został wciśnięty
        } else if (ipButton.getGlobalBounds().contains(mousePos)) {
            ipButtonPressed = true;
            return true; // Sygnalizowanie, że przycisk wysyłania IP został wciśnięty
        }
    }

    if (event.type == sf::Event::TextEntered) {
        // Obsługa aktywnego input boxa
        if (activeInput == InputType::Nick) {
            if (event.text.unicode == 8 && !inputText.empty()) { // Backspace
                inputText.pop_back();
            } else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                inputText += static_cast<char>(event.text.unicode);
            }
        } else if (activeInput == InputType::IP) {
            if (event.text.unicode == 8 && !ipText.empty()) { // Backspace
                ipText.pop_back();
            } else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                ipText += static_cast<char>(event.text.unicode);
            }
        }
    }

    return false;
}

void Widok_Login::render(bool already_connected) {
    window.draw(title);

    // Renderowanie pola nicku
    window.draw(inputBoxNick);
    sf::Text nickDisplay(inputText, Resources::getFont(), 20);
    nickDisplay.setFillColor(sf::Color::Black);
    nickDisplay.setPosition(310, 410);
    window.draw(nickDisplay);

    // Renderowanie pola IP
    window.draw(inputBoxIP);
    sf::Text ipDisplay(ipText, Resources::getFont(), 20);
    ipDisplay.setFillColor(sf::Color::Black);
    ipDisplay.setPosition(310, 130);
    window.draw(ipDisplay);

    // Renderowanie przycisku
    if (already_connected == true)
    {
        window.draw(button);
        window.draw(buttonLabel);
    }
    
 
    window.draw(ipButton);
    window.draw(ipButtonLabel);

    // Renderowanie komunikatu o błędzie, jeśli aktywny
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


void Widok_Login::resetIPButton() {
    ipButtonPressed = false;
}

void Widok_Login::resetButton() {
    buttonPressed = false;
}



