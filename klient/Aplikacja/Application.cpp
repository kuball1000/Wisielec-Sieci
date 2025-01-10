#include "Application.h"
#include <iostream>
#include <arpa/inet.h>

Application::Application()
    : window(sf::VideoMode(800, 600), "SFML Application"),
      currentView(ViewState::Login),
      loginView(window),
      choiceView(window),
      gameView(window) {
        if (!connectToServer("127.0.0.1", 12345)) {
            std::cerr << "Nie udało się połączyć z serwerem." << std::endl;
        }
      }

bool Application::connectToServer(const std::string& serverIp, unsigned short port) {
    if (tcpSocket.connect(serverIp, port) == sf::Socket::Done) {
        std::cout << "Połączono z serwerem na " << serverIp << ":" << port << std::endl;
        return true;
    }
    return false;
}

bool Application::sendMessage(const std::string& message) {
    uint32_t len = htonl(message.size());
    if (tcpSocket.send(&len, sizeof(len)) != sf::Socket::Done) {
        return false;
    }
    if (tcpSocket.send(message.c_str(), message.size()) != sf::Socket::Done) {
        return false;
    }
    return true;
}

void Application::run() {
    while (window.isOpen()) {
        handleEvents();
        render();
    }
}

void Application::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        switch (currentView) {
            case ViewState::Login:
                if (loginView.handleEvent(event)) {
                    if (!sendMessage(loginView.getNick())) {
                        std::cerr << "Błąd wysyłania nicka." << std::endl;
                    }
                    currentView = ViewState::Choice1;
                }
                break;
            case ViewState::Choice1:
                if (choiceView.handleEvent(event, currentRoom)) {
                    // Przechodzenie między widokami w ramach Widok_Choice
                    if (choiceView.needsReset()) {
                        choiceView.resetToMainView();
                    } else if (!currentRoom.empty()) {
                        currentView = ViewState::Game;
                    }
                }
                break;
            case ViewState::Game:
                if (gameView.handleEvent(event)) {
                    currentView = ViewState::Choice1;
                }
                break;
        }
    }
}

void Application::render() {
    window.clear(sf::Color(200, 200, 200));

    switch (currentView) {
        case ViewState::Login:
            loginView.render();
            break;
        case ViewState::Choice1:
            choiceView.render();
            break;
        case ViewState::Game:
            gameView.render(currentRoom);
            break;
    }

    window.display();
}
