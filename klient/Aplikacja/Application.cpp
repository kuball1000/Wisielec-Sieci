#include "Application.h"
#include <iostream>
#include <arpa/inet.h>
#include "Widok_Choice.h"


Application::Application()
    : window(sf::VideoMode(800, 600), "SFML Application"),
      currentView(ViewState::Login),
      loginView(window),
      choiceView(window, this),
      gameView(window),
      password("______"), // Przykładowe hasło
      playerNames({"Gracz1", "Gracz2", "Gracz3", "Gracz4"}),
      playerStages({4, 5, 6, 3}),
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

bool Application::connectToServer(const std::string &serverIp, unsigned short port) {
    if (tcpSocket.connect(serverIp, port) == sf::Socket::Done) {
        std::cout << "Połączono z serwerem na " << serverIp << ":" << port << std::endl;
        return true;
    }
    return false;
}

bool Application::sendMessage(const std::string &message) {
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
            {
            std::lock_guard<std::mutex> lock(messageMutex); // Zablokuj mutex
            lastMessage = message; // Zaktualizuj wiadomość
            }
            std::cout << "Serwer: " << message << std::endl;
            waitingForResponse = false;
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
                    } else {

                        waitingForResponse = true;

                        while (waitingForResponse) {
                            sf::sleep(sf::milliseconds(10)); // Mała przerwa, aby odciążyć CPU
                        }

                        std::string message;
                        {
                            std::lock_guard<std::mutex> lock(messageMutex); // Zablokuj mutex
                            message = lastMessage;
                        }

                        // Oczekuj odpowiedzi serwera
                        std::cerr << lastMessage << std::endl;
                        if (lastMessage == "Nick jest już zajęty. Podaj inny:") {
                        } else {
                            currentView = ViewState::Choice1;
                        }
                    }
                }
                break;
        case ViewState::Choice1:
            if (choiceView.handleEvent(event, currentRoom)) {
                Widok_Choice::State choiceState = choiceView.getCurrentState();
                std::cerr << static_cast<int>(choiceState) << std::endl;
                if (choiceState == Widok_Choice::State::CreateRoomView) {
                    if (!sendMessage("1")) {
                        std::cerr << "Błąd wysyłania wiadomości o tworzeniu pokoju." << std::endl;
                    }
                } else if (choiceState == Widok_Choice::State::JoinRoomView) {
                    if (!sendMessage("2")) {
                        std::cerr << "Błąd wysyłania wiadomości o dołączaniu do pokoju." << std::endl;
                    } else {
                        waitingForResponse = true;
                        while (waitingForResponse) {
                            sf::sleep(sf::milliseconds(10));
                        }
                        {
                            std::lock_guard<std::mutex> lock(messageMutex);
                            std::cerr << "Odpowiedź serwera: " << lastMessage << std::endl;

                            // Przetwarzanie odpowiedzi serwera
                            choiceView.parseAvailableRooms(lastMessage);
                        }

                    }
                }

                if (choiceView.needsReset()) {
                    // Powrót do głównego widoku z Widok_Choice
                    choiceView.resetToMainView();
                } else if (!currentRoom.empty()) {
                    // Przejście do widoku gry po wybraniu pokoju
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
        gameView.render(currentRoom, password, playerNames, playerStages);
        break;
    }

    window.display();
}
