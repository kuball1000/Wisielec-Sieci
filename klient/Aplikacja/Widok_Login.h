#ifndef WIDOK_LOGIN_H
#define WIDOK_LOGIN_H

#include <SFML/Graphics.hpp>
#include "Resources.h"
#include <string>
#include <chrono>

class Widok_Login {
public:
    explicit Widok_Login(sf::RenderWindow& window);

    bool handleEvent(const sf::Event& event);
    void render(bool already_connected);
    void showErrorMessage(const std::string& message);
    bool isIPButtonPressed() const { return ipButtonPressed; }
    void resetIPButton();

    bool isButtonPressed() const { return buttonPressed; }
    void resetButton();

    const std::string& getNick() const { return inputText; }
    const std::string& getIP() const { return ipText; }

private:
    sf::RenderWindow& window;
      bool buttonPressed;

    // Tytuł
    sf::Text title;
    //
    sf::Text titlenick;

  
    // Aktywny input box
    enum class InputType { Nick, IP };
    InputType activeInput = InputType::Nick;

    // IP input box
    sf::Text inputLabelIP;
    sf::RectangleShape inputBoxIP;
    std::string ipText;
    
    // Nick input box
    sf::Text inputLabelNick;
    sf::RectangleShape inputBoxNick;
    std::string inputText;
    // Przycisk "Zaloguj"
    sf::RectangleShape button;
    sf::Text buttonLabel;

    // Komunikat o błędzie
    sf::Text errorMessage;
    std::chrono::steady_clock::time_point errorMessageTimer;

   

    sf::RectangleShape ipButton;       // Nowy przycisk
    sf::Text ipButtonLabel;            // Etykieta nowego przycisku
    bool ipButtonPressed = false;      // Flaga dla wciśnięcia przycisku
};

#endif
