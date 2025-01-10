#include "Application.h"
#include <iostream>
#include <arpa/inet.h>

Application::Application()
    : window(sf::VideoMode(800, 600), "SFML Application"),
      currentView(ViewState::Login),
      loginView(window),
      choiceView(window),
      gameView(window),
      running(true) {
        if (!connectToServer("127.0.0.1", 12345)) {
            std::cerr << "Nie udało się połączyć z serwerem." << std::endl;
        } else {
        // Uruchom wątek odbierania wiadomości
        receiverThread = std::thread(&Application::receiveMessages, this);
        }
    }

Application::~Application() {
    // Zatrzymaj wątek odbierania wiadomości
    running = false;
    if (receiverThread.joinable()) {
        receiverThread.join();
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

void Application::receiveMessages() {
    while (running) {
        std::string message;
        uint32_t len = 0;
        std::size_t received = 0;

        // Odbierz długość wiadomości
        sf::Socket::Status status = tcpSocket.receive(&len, sizeof(len), received);

        if (status == sf::Socket::Disconnected) {
            std::cerr << "Połączenie z serwerem zostało zamknięte." << std::endl;
            running = false; // Wyłącz odbiornik
            break;
        } else if (status != sf::Socket::Done || received != sizeof(len)) {
            if (running) { // Jeśli flaga jest aktywna, zgłoś błąd
                std::cerr << "Błąd podczas odbierania długości wiadomości." << std::endl;
            }
            continue;
        }

        len = ntohl(len);

        // Odbierz treść wiadomości
        char buffer[1024];
        message.clear();
        while (len > 0) {
            std::size_t bytesReceived = 0;
            status = tcpSocket.receive(buffer, std::min(len, static_cast<uint32_t>(sizeof(buffer))), bytesReceived);

            if (status == sf::Socket::Disconnected) {
                std::cerr << "Połączenie z serwerem zostało zamknięte." << std::endl;
                running = false; // Wyłącz odbiornik
                break;
            } else if (status != sf::Socket::Done) {
                std::cerr << "Błąd podczas odbierania wiadomości." << std::endl;
                break;
            }
            
            message.append(buffer, bytesReceived);
            len -= bytesReceived;
        }

        if (!message.empty()) {
            std::cout << "Serwer: " << message << std::endl;
        }
    }
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
                    currentView = currentRoom.empty() ? ViewState::Choice1 : ViewState::Game;
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
