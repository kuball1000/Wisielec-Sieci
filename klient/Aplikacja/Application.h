#ifndef APPLICATION_H
#define APPLICATION_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <thread>
#include <atomic>
#include "Resources.h"
#include "Widok_Login.h"
#include "Widok_Choice.h"
#include "Widok_Game.h"
#include <mutex>
#include <vector>
#include <string>
#include <queue>

class Application {
public:
    Application();
    ~Application();
    void run();

    bool sendMessage(const std::string& message);
    void receiveMessages();
    void updatePlayerList(const std::string &serverMessage);
    void  parseServerMessage(const std::string &message);
private:
    void handleEvents();
    void render();
    bool connectToServer(const std::string& serverIp, unsigned short port);
    bool lobbyflag;
    bool playerStagesInitialized = false;

    std::queue<std::function<void()>> renderQueue; // Kolejka zdarzeń do renderowania
    std::mutex renderQueueMutex; // Mutex do synchronizacji kolejki


    sf::RenderWindow window;
    enum class ViewState { Login, Choice1, Lobby, Game } currentView;

    Widok_Login loginView;
    Widok_Choice choiceView;
    Widok_Game gameView;

    sf::TcpSocket tcpSocket;
    std::thread receiverThread; // Wątek do odbierania wiadomości
    std::atomic<bool> running;
    
    std::string currentRoom;
    std::string lastMessage;
    std::mutex messageMutex;
    std::atomic<bool> waitingForResponse = false; // Flaga oczekiwania na odpowiedź


    // Zmienne dla widoku gry
    std::string password; // Aktualne hasło w grze
    std::vector<std::string> playerNames; // Nazwy graczy w pokoju
    std::vector<int> playerStages; // Etapy wisielca dla każdego gracza
    std::string usedLetters;                  // Przechowuje niepoprawne litery
    int lives;                                // Liczba pozostałych prób
    std::vector<int> playerScores;            // Lista wyników graczy
    std::string serverMessages; 
};

#endif