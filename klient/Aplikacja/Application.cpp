#include "Application.h"
#include <iostream>
#include <arpa/inet.h>
#include "Widok_Choice.h"
#include "Widok_Login.h"
#include <sstream>


Application::Application()
    : window(sf::VideoMode(800, 600), "SFML Application"),
      currentView(ViewState::Login),
      loginView(window),
      choiceView(window, this),
      gameView(window, this),
      password(""), // Przykładowe hasło
      playerNames({}),
      running(true),
      playerStages({0,0,0,0})
      
{
    while (currentView == ViewState::Login) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return; // Wyjdź z aplikacji
            }

            if (loginView.handleEvent(event)) {
                if (loginView.isIPButtonPressed()) {
                // Użytkownik kliknął przycisk logowania
                std::string serverIp = loginView.getIP();
                loginView.resetIPButton();
                std::cout << "Próba połączenia z serwerem: " << serverIp << std::endl;
                

                if (!connectToServer(serverIp, 12345)) {
                    loginView.showErrorMessage("Nie udalo się polaczyc z serwerem: " + serverIp);
                } else {
                    std::cout << "Połączono z serwerem." << std::endl;
                    already_connected = true;
                    receiverThread = std::thread(&Application::receiveMessages, this);
                    
                }

    

                }

                if (loginView.isButtonPressed()) {
                    loginView.resetButton();

                if (!sendMessage(loginView.getNick()))
                {
                    std::cerr << "Błąd wysyłania nicka." << std::endl;
                }
                else
                {

                    waitingForResponse = true;

                    while (waitingForResponse)
                    {
                        sf::sleep(sf::milliseconds(10)); // Mała przerwa, aby odciążyć CPU
                    }

                    std::string message;
                    {
                        std::lock_guard<std::mutex> lock(messageMutex); // Zablokuj mutex
                        message = lastMessage;
                    }

                    // Oczekuj odpowiedzi serwera
                    std::cerr << lastMessage << std::endl;
                    if (lastMessage == "Nick jest już zajęty. Podaj inny:")
                    {
                        loginView.showErrorMessage("Nick jest juz zajety. Podaj inny");
                    }
                    else
                    {
                        currentView = ViewState::Choice1;
                    }
                }
                }
            }
        }

        window.clear(sf::Color(200, 200, 200));
        loginView.render(already_connected);
        window.display();
    }


}

Application::~Application()
{
    // Zatrzymaj wątek odbierania wiadomości
    running = false;
    if (receiverThread.joinable())
    {
        receiverThread.join();
    }
}

bool Application::connectToServer(const std::string &serverIp, unsigned short port)
{
    if (tcpSocket.connect(serverIp, port) == sf::Socket::Done)
    {
        std::cout << "Połączono z serwerem na " << serverIp << ":" << port << std::endl;
        return true;
    }
    return false;
}

bool Application::sendMessage(const std::string &message)
{
    uint32_t len = htonl(message.size());
    if (tcpSocket.send(&len, sizeof(len)) != sf::Socket::Done)
    {
        return false;
    }
    if (tcpSocket.send(message.c_str(), message.size()) != sf::Socket::Done)
    {
        return false;
    }
    return true;
}

void Application::receiveMessages()
{
    while (running)
    {
        std::string message;
        uint32_t len = 0;
        std::size_t received = 0;

        // Odbierz długość wiadomości
        sf::Socket::Status status = tcpSocket.receive(&len, sizeof(len), received);

        if (status == sf::Socket::Disconnected)
        {
            std::cerr << "Połączenie z serwerem zostało zamknięte." << std::endl;
            running = false; // Wyłącz odbiornik
            break;
        }
        else if (status != sf::Socket::Done || received != sizeof(len))
        {
            if (running)
            { // Jeśli flaga jest aktywna, zgłoś błąd
                std::cerr << "Błąd podczas odbierania długości wiadomości." << std::endl;
            }
            continue;
        }

        len = ntohl(len);

        // Odbierz treść wiadomości
        char buffer[1024];
        message.clear();
        while (len > 0)
        {
            std::size_t bytesReceived = 0;
            status = tcpSocket.receive(buffer, std::min(len, static_cast<uint32_t>(sizeof(buffer))), bytesReceived);

            if (status == sf::Socket::Disconnected)
            {
                std::cerr << "Połączenie z serwerem zostało zamknięte." << std::endl;
                running = false; // Wyłącz odbiornik
                break;
            }
            else if (status != sf::Socket::Done)
            {
                std::cerr << "Błąd podczas odbierania wiadomości." << std::endl;
                break;
            }

            message.append(buffer, bytesReceived);
            len -= bytesReceived;
        }

        if (!message.empty()) {
            {
                std::lock_guard<std::mutex> lock(messageMutex); // Zablokuj mutex
                lastMessage = message;                          // Zaktualizuj wiadomość
            }
            std::cout << "Serwer: " << message << std::endl;
            if (message.find("Gracze w pokoju:") != std::string::npos ) { //current view nie dziala ale tak moze zostac imo
                    std::cout << "Aktualizowanie listy graczy..." << std::endl;

                updatePlayerList(message); // Zaktualizuj listę graczy
            }

            if (message.find("Gra rozpoczyna się!") != std::string::npos ) { //current view nie dziala ale tak moze zostac imo
                std::cout << "Gra została rozpoczęta!" << std::endl;
                currentView = ViewState::Game; // Przejdź do widoku gry
            }

            if (currentView == ViewState::Game) { //current view nie dziala ale tak moze zostac imo
                parseServerMessage(message);
            }

            // Czekanie na zakończenie rundy w trwającym w pokoju grze i dołączneie do niej później
            if(message.find("Niepoprawne litery:") != std::string::npos && currentView == ViewState::Lobby) {
                currentView = ViewState::Game;
            }
        
            waitingForResponse = false;
        }
    }
}

void Application::run()
{
    while (window.isOpen())
    {
        handleEvents();
        render();
    }
}

void Application::handleEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed) {
            window.close();
            if (!sendMessage("/exit")) {
                std::cerr << "Błąd wysyłania wiadomości /exit do serwera." << std::endl;
            }
        }

        switch (currentView) {
        case ViewState ::Login:
            break;
        case ViewState::Choice1:
            if (choiceView.handleEvent(event, currentRoom)) {
                Widok_Choice::State choiceState = choiceView.getCurrentState();
                if (choiceState == Widok_Choice::State::CreateRoomView) {
                    lobbyflag = true;
                    if (!sendMessage("1")) {
                        std::cerr << "Błąd wysyłania wiadomości o tworzeniu pokoju." << std::endl;
                    }
                } else if (choiceState == Widok_Choice::State::JoinRoomView) {
                    lobbyflag = false;
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
                }
                else if (!currentRoom.empty())
                {
                        std::cout << "Przejście do Lobby. Pokój: " << currentRoom << std::endl;

                    currentView = ViewState::Lobby; // Przejście do Lobby po stworzeniu pokoju
                }
            }
            break;

        case ViewState::Lobby:
            if (gameView.handleLobbyEvent(event))
            {                                  // Obsługa zdarzeń w Lobby
                if (!sendMessage("/start")) {
                    std::cerr << "Błąd wysyłania wiadomości o startu gry." << std::endl;
                }
                currentView = ViewState::Game; // Przejście do widoku gry po kliknięciu "Start Game"
            }
            else if (gameView.handleLobbyEvent(event))
            {
                currentView = ViewState::Choice1; // Powrót do widoku wyboru
            }
            break;
        case ViewState::Game:
            if (gameView.handleGameEvent(event))
            {
                currentView = ViewState::Choice1;
            }
            break;
        }
    }
}

void Application::render()
{
    window.clear(sf::Color(200, 200, 200));

    switch (currentView)
    {
    case ViewState::Login:
        loginView.render(already_connected);
        break;
    case ViewState::Choice1:
        choiceView.render();
        break;
    case ViewState::Game:
        if (!playerStagesInitialized) {
            playerStages.clear(); // Wyczyść etapy graczy
            playerStages.resize(playerNames.size(), 0); // Ustaw zerowe etapy dla każdego gracza
            usedLetters.clear(); // Wyczyść użyte litery
            playerStagesInitialized = true; // Oznacz jako zainicjalizowane
        }
        gameView.renderGame(currentRoom, password, usedLetters, lives, playerNames, playerStages, _serverMessages);
        break;
    case ViewState::Lobby:
        gameView.renderLobby(playerNames,lobbyflag,currentRoom);
        break;
    }

    window.display();
}

void Application::updatePlayerList(const std::string &serverMessage)
{
    playerNames.clear(); // Wyczyść listę graczy

    std::istringstream stream(serverMessage);
    std::string line;

    // Pomijamy linię "Gracze w pokoju:"
    std::getline(stream, line);

    while (std::getline(stream, line))
    {
        if (!line.empty())
        {
            playerNames.push_back(line); // Dodaj każdego gracza do listy
        }
    }
}

void Application::parseServerMessage(const std::string &message)
{
    std::istringstream stream(message);
    std::string line;

    bool needsRender = false;

    while (std::getline(stream, line))
    {
        if (line.find("Hasło:") != std::string::npos)
        {
            this->password = line.substr(line.find(":") + 2);
            needsRender = true;
        }
        else if (line.find("Niepoprawne litery:") != std::string::npos)
        {
            this->usedLetters = line.substr(line.find(":") + 2);
            needsRender = true;
        }
        else if (line.find("Pozostałe próby:") != std::string::npos)
        {
            this->lives = std::stoi(line.substr(line.find(":") + 2));
            needsRender = true;
        }
        else if (line.find("Stan graczy w pokoju:") != std::string::npos) {

    // Pomiń pierwszą linię
    while (std::getline(stream, line)) {

        // Podział na nick i wynik
        size_t separator = line.find(',');
        if (separator != std::string::npos) {
            std::string playerName = line.substr(0, separator);
            int playerStage = std::stoi(line.substr(separator + 1));

            // Aktualizacja listy graczy i ich etapów
            auto it = std::find(playerNames.begin(), playerNames.end(), playerName);
            if (it != playerNames.end()) {
                // Gracz istnieje – aktualizujemy jego etap
                size_t index = std::distance(playerNames.begin(), it);
                playerStages[index] = playerStage;
            } else {
                // Gracz nie istnieje – dodajemy nowego
                playerNames.push_back(playerName);
                playerStages.push_back(playerStage);
            }
        }
    }
    needsRender = true;
    } else if (line.find("Czas się skończył! Gra zakończona.") != std::string::npos) {
        playerStagesInitialized = false;
        needsRender = true;
        _serverMessages = "Czas sie skonczyl! Gra rozpoczyna sie od nowa!";
    } else if (line.find("Nowa gra rozpoczyna się! Hasło:") != std::string::npos) {
        playerStagesInitialized = false;
        needsRender = true;
        _serverMessages = "Nowa gra rozpoczyna się!";
    } else if (line.find("Już zgadywałeś tę literę. Podaj inną literę: ") != std::string::npos) {
        _serverMessages = "Juz zgadywales te litere!";
        needsRender = true;
    } else if (line.find("Odgadłeś hasło! Wygrałeś.") != std::string::npos) {
        _serverMessages = "Odgadles haslo! Wygrales! Nowa gra zaraz sie rozpocznie!";
        playerStagesInitialized = false;
        needsRender = true;
    } else if (line.find("Gracz ") != std::string::npos && line.find(" odgadl haslo: ") != std::string::npos) {
            playerStagesInitialized = false;

            _serverMessages = line + " Nowa gra zaraz sie rozpocznie!";
            needsRender = true;
    } else if (line.find("Zostałeś ostatnim graczem w pokoju. Wygrywasz.") != std::string::npos) {
            playerStagesInitialized = false;

            _serverMessages = "Zostales ostatnim graczem w pokoju, nastepuje rozlaczanie...";
            sf::sleep(sf::milliseconds(5000));


            window.close();
            if (!sendMessage("/exit")) {
                std::cerr << "Błąd wysyłania wiadomości /exit do serwera." << std::endl;
            }
    } else if (line.find("Przegrałeś! Hasło to: ") != std::string::npos) {
            playerStagesInitialized = false;

            _serverMessages = "Przegrales! Zuzyles wszystkie zycia!";
            needsRender = true;
    } else if (line.find("Zostałeś ostatnim graczem przy życiu! Wygrywasz.") != std::string::npos) {
            playerStagesInitialized = false;

            _serverMessages = "Zostales ostatnim graczem przy zyciu! Wygrywasz.";
            needsRender = true;
    }





    }
    if (needsRender && currentView == ViewState::Game) {
        std::lock_guard<std::mutex> lock(renderQueueMutex);
        renderQueue.push([this]() {
            render();
        });
    }

    // Aktualizacja serwerowych wiadomości
    serverMessages = message;

}
