// Serwer w C++ dla gry "Wisielec"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

const int PORT = 12345;
const int MAX_CLIENTS = 4;
const int MAX_WRONG_GUESSES = 6;

std::mutex clients_mutex;
std::unordered_map<int, std::string> clients;
std::vector<std::string> word_pool = {"example", "network", "hangman", "server", "client"};

void broadcast(const std::string& message, int exclude_socket = -1) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& [socket, nick] : clients) {
        if (socket != exclude_socket) {
            send(socket, message.c_str(), message.length(), 0);
        }
    }
}

void handle_client(int client_socket) {
    char buffer[1024];
    std::string client_nick;

    // Odbierz nick od klienta
    send(client_socket, "Podaj swój nick: ", 18, 0);
    int len = recv(client_socket, buffer, 1024, 0);
    if (len <= 0) {
        close(client_socket);
        return;
    }
    buffer[len] = '\0';
    client_nick = buffer;

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_socket] = client_nick;
    }

    std::cout << client_nick << " dołączył do gry." << std::endl;
    broadcast(client_nick + " dołączył do gry.\n", client_socket);

    // Rozpocznij grę
    std::string secret_word = word_pool[rand() % word_pool.size()];
    std::string guessed_word(secret_word.size(), '_');
    std::string wrong_guesses;
    int wrong_count = 0;

    while (true) {
        // Wyślij stan gry
        std::string game_state = "Hasło: " + guessed_word + "\n";
        game_state += "Niepoprawne litery: " + wrong_guesses + "\n";
        game_state += "Pozostałe próby: " + std::to_string(MAX_WRONG_GUESSES - wrong_count) + "\n";
        send(client_socket, game_state.c_str(), game_state.length(), 0);

        // Pobierz literę od gracza
        send(client_socket, "Podaj literę: ", 14, 0);
        len = recv(client_socket, buffer, 1024, 0);
        if (len <= 0) break;

        buffer[len] = '\0';
        char guessed_char = buffer[0];

        // Sprawdź literę
        bool correct_guess = false;
        for (size_t i = 0; i < secret_word.size(); ++i) {
            if (secret_word[i] == guessed_char) {
                guessed_word[i] = guessed_char;
                correct_guess = true;
            }
        }

        if (!correct_guess) {
            wrong_guesses += guessed_char;
            wrong_guesses += ' ';
            wrong_count++;
        }

        // Sprawdź warunki zakończenia gry
        if (guessed_word == secret_word) {
            send(client_socket, "Gratulacje! Wygrałeś!\n", 26, 0);
            break;
        }
        if (wrong_count >= MAX_WRONG_GUESSES) {
            send(client_socket, ("Przegrałeś! Hasło to: " + secret_word + "\n").c_str(), secret_word.size() + 20, 0);
            break;
        }
    }

    // Usuń klienta
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(client_socket);
    }
    close(client_socket);
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Nie można utworzyć gniazda." << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Nie można związać gniazda z adresem." << std::endl;
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        std::cerr << "Błąd nasłuchiwania." << std::endl;
        return 1;
    }

    std::cout << "Serwer uruchomiony na porcie " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

        if (client_socket == -1) {
            std::cerr << "Błąd akceptacji połączenia." << std::endl;
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}
