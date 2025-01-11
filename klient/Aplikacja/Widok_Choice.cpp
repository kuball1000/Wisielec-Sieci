#include "Widok_Choice.h"
#include "Resources.h"
#include "Application.h" 
#include <iostream>
#include <sstream>


Widok_Choice::Widok_Choice(sf::RenderWindow &window, Application* app) : window(window), application(app), currentState(State::MainView)
{
    // Główny widok
    title.setFont(Resources::getFont());
    title.setString("Wybierz opcje:");
    title.setCharacterSize(24);
    title.setPosition(300, 50);
    title.setFillColor(sf::Color::Black);

    createButton.setSize(sf::Vector2f(200, 40));
    createButton.setFillColor(sf::Color(100, 149, 237));
    createButton.setPosition(300, 150);

    createButtonText.setFont(Resources::getFont());
    createButtonText.setString("Stworz nowy pokoj");
    createButtonText.setCharacterSize(20);
    createButtonText.setFillColor(sf::Color::White);
    createButtonText.setPosition(310, 160);

    joinButton.setSize(sf::Vector2f(200, 40));
    joinButton.setFillColor(sf::Color(100, 149, 237));
    joinButton.setPosition(300, 220);

    joinButtonText.setFont(Resources::getFont());
    joinButtonText.setString("Dolacz do pokoju");
    joinButtonText.setCharacterSize(20);
    joinButtonText.setFillColor(sf::Color::White);
    joinButtonText.setPosition(310, 230);

    // Widok "Stwórz pokój"
    inputBox.setSize(sf::Vector2f(300, 40));
    inputBox.setFillColor(sf::Color(230, 230, 230));
    inputBox.setPosition(250, 150);

    inputText.setFont(Resources::getFont());
    inputText.setCharacterSize(20);
    inputText.setFillColor(sf::Color::Black);
    inputText.setPosition(260, 160);

    confirmButton.setSize(sf::Vector2f(200, 40));
    confirmButton.setFillColor(sf::Color(0, 128, 0));
    confirmButton.setPosition(300, 220);

    confirmButtonText.setFont(Resources::getFont());
    confirmButtonText.setString("Stworz pokoj");
    confirmButtonText.setCharacterSize(20);
    confirmButtonText.setFillColor(sf::Color::White);
    confirmButtonText.setPosition(310, 230);

    // Przycisk powrotu
    backButton.setSize(sf::Vector2f(100, 40));
    backButton.setFillColor(sf::Color(128, 128, 128));
    backButton.setPosition(50, 500);

    backButtonText.setFont(Resources::getFont());
    backButtonText.setString("Powrot");
    backButtonText.setCharacterSize(20);
    backButtonText.setFillColor(sf::Color::White);
    backButtonText.setPosition(60, 510);
}

void Widok_Choice::addRoom(const std::string &roomName)
{
    roomNames.push_back(roomName);
}

bool Widok_Choice::handleEvent(const sf::Event &event, std::string &roomName)
{
    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

    if (currentState == State::MainView)
    {
        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (createButton.getGlobalBounds().contains(mousePos))
            {
                currentState = State::CreateRoomView;
                return true;
            }
            else if (joinButton.getGlobalBounds().contains(mousePos))
            {
                currentState = State::JoinRoomView;
                return true;
            }
        }
    }
    else if (currentState == State::CreateRoomView)
    {
        if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode == 8 && !roomNameInput.empty())
            { // Backspace
                roomNameInput.pop_back();
            }
            else if (event.text.unicode >= 32 && event.text.unicode <= 126)
            {
                roomNameInput += static_cast<char>(event.text.unicode);
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            if (confirmButton.getGlobalBounds().contains(mousePos))
            {
                roomName = roomNameInput;
                addRoom(roomName); // Dodajemy nowy pokój do listy
                if (!application->sendMessage(roomName)) {
                    // std::cerr << "Błąd wysyłania nazwy pokoju na serwer: " << roomName << std::endl;
                } else {
                    // std::cout << "Wysłano nazwę pokoju: " << roomName << " na serwer." << std::endl;
                }

                return true;       // Przejście do widoku gry
            }
            else if (backButton.getGlobalBounds().contains(mousePos))
            {
                currentState = State::MainView;
            }
        }
    }
    else if (currentState == State::JoinRoomView)
    {
        if (event.type == sf::Event::MouseButtonPressed)
        {
            // Sprawdzenie kliknięcia na przycisk pokoju
            for (size_t i = 0; i < roomButtons.size(); ++i)
            {
                if (roomButtons[i].getGlobalBounds().contains(mousePos))
                {
                    roomName = roomNames[i];

                    // Usunięcie wszystkiego po pierwszej spacji lub nawiasie
                    size_t pos = roomName.find_first_of(" (");
                    if (pos != std::string::npos) {
                        roomName = roomName.substr(0, pos);
                    }

                    if (!application->sendMessage(roomName)) {
                        std::cerr << "Błąd wysyłania nazwy pokoju na serwer: " << roomName << std::endl;
                    } else {
                        std::cout << "Wysłano nazwę pokoju: " << roomName << " na serwer." << std::endl;
                    }

                    return true; // Przejście do widoku gry
                }
            }
            // Obsługa przycisku powrotu
            if (backButton.getGlobalBounds().contains(mousePos))
            {
                currentState = State::MainView;
            }
        }
    }
    return false;
}

void Widok_Choice::render()
{
    if (currentState == State::MainView)
    {
        window.draw(title);
        window.draw(createButton);
        window.draw(createButtonText);
        window.draw(joinButton);
        window.draw(joinButtonText);
    }
    else if (currentState == State::CreateRoomView)
    {
        renderCreateRoom();
    }
    else if (currentState == State::JoinRoomView)
    {
        renderJoinRoom();
    }
}

void Widok_Choice::renderCreateRoom()
{
    window.draw(inputBox);

    sf::Text displayedInput(roomNameInput, Resources::getFont(), 20);
    displayedInput.setPosition(260, 160);
    displayedInput.setFillColor(sf::Color::Black);
    displayedInput.setString(roomNameInput);

    window.draw(displayedInput);
    window.draw(confirmButton);
    window.draw(confirmButtonText);
    window.draw(backButton);
    window.draw(backButtonText);
}

void Widok_Choice::renderJoinRoom()
{
    roomButtons.clear();
    roomButtonTexts.clear();

    for (size_t i = 0; i < roomNames.size(); ++i)
    {
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

    for (size_t i = 0; i < roomButtons.size(); ++i)
    {
        window.draw(roomButtons[i]);
        window.draw(roomButtonTexts[i]);
    }

    window.draw(backButton);
    window.draw(backButtonText);
}

bool Widok_Choice::needsReset() const
{
    return currentState == State::MainView;
}

void Widok_Choice::resetToMainView()
{
    currentState = State::MainView;
    roomNameInput.clear();
}

void Widok_Choice::parseAvailableRooms(const std::string &serverResponse) {
    roomNames.clear(); // Wyczyść listę pokoi przed dodaniem nowych

    std::istringstream stream(serverResponse);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("Podaj nazwę pokoju") != std::string::npos) {
            break; // Koniec listy pokoi
        }

        // Dodaj linię do listy pokoi (usuń ewentualne białe znaki na końcu)
        roomNames.push_back(line);

    }
}
