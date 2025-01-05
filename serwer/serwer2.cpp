#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <condition_variable>
#include <chrono>

const int PORT = 12345;
const int MAX_CLIENTS = 4;
const int MAX_WRONG_GUESSES = 6;

std::mutex clients_mutex;
std::unordered_map<int, std::string> clients;
std::unordered_map<std::string, std::unordered_set<int>> rooms;
std::vector<std::string> word_pool = {"example", "network", "hangman", "server", "client"};

// Struktura przechowująca wspólne hasło i stany graczy
struct GameState {
    std::string secret_word; // Wspólne hasło
    std::unordered_map<int, std::string> guessed_words; // Indywidualne zgadywane słowa
    std::unordered_map<int, std::string> wrong_guesses; // Indywidualne błędne litery
    std::unordered_map<int, int> wrong_counts;         // Indywidualna liczba błędów
    std::mutex game_mutex;
    bool game_over = false;  // Flaga oznaczająca koniec gry
};

// Globalne zmienne
std::unordered_map<std::string, std::shared_ptr<GameState>> room_states;
std::unordered_map<std::string, std::shared_ptr<std::condition_variable>> room_conditions;
std::unordered_map<std::string, std::shared_ptr<std::mutex>> room_mutexes;

void broadcast_to_room(const std::string& room_name, const std::string& message, int exclude_socket = -1) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int socket : rooms[room_name]) {
        if (socket != exclude_socket) {
            send(socket, message.c_str(), message.length(), 0);
        }
    }
}

void send_game_state_to_client(const std::string& room_name, int client_socket) {
    auto game_state = room_states[room_name];
    auto& guessed_word = game_state->guessed_words[client_socket];
    auto& wrong_guesses = game_state->wrong_guesses[client_socket];
    auto& wrong_count = game_state->wrong_counts[client_socket];

    std::string game_state_msg = "Hasło: " + guessed_word + "\n";
    game_state_msg += "Niepoprawne litery: " + wrong_guesses + "\n";
    game_state_msg += "Pozostałe próby: " + std::to_string(MAX_WRONG_GUESSES - wrong_count) + "\n";
    send(client_socket, game_state_msg.c_str(), game_state_msg.length(), 0);
}

void restart_game(const std::string& room_name) {
    auto game_state = room_states[room_name];
    {
        std::lock_guard<std::mutex> lock(game_state->game_mutex);
        game_state->secret_word = word_pool[rand() % word_pool.size()];
        game_state->game_over = false;
        for (int socket : rooms[room_name]) {
            game_state->guessed_words[socket] = std::string(game_state->secret_word.size(), '_');
            game_state->wrong_guesses[socket].clear();
            game_state->wrong_counts[socket] = 0;
        }
    }
    broadcast_to_room(room_name, "Nowa gra rozpoczyna się!");
    for (int socket : rooms[room_name]) {
        send_game_state_to_client(room_name, socket);
    }
    room_conditions[room_name]->notify_all();
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

    std::cout << client_nick << " dołączył do serwera." << std::endl;
    send(client_socket, "Witaj! Możesz stworzyć pokój (1) lub dołączyć do istniejącego (2): ", 88, 0);

    len = recv(client_socket, buffer, 1024, 0);
    if (len <= 0) {
        close(client_socket);
        return;
    }
    buffer[len] = '\0';
    int choice = buffer[0] - '0';
    std::string room_name;

    if (choice == 1) {
        send(client_socket, "Podaj nazwę pokoju: ", 21, 0);
        len = recv(client_socket, buffer, 1024, 0);
        if (len <= 0) {
            close(client_socket);
            return;
        }
        buffer[len] = '\0';
        room_name = buffer;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            rooms[room_name].insert(client_socket);
        }

        // Inicjalizacja stanu gry
        auto game_state = std::make_shared<GameState>();
        game_state->secret_word = word_pool[rand() % word_pool.size()];
        game_state->guessed_words[client_socket] = std::string(game_state->secret_word.size(), '_');
        game_state->wrong_counts[client_socket] = 0;
        room_states[room_name] = game_state;

        // Inicjalizacja synchronizacji
        room_conditions[room_name] = std::make_shared<std::condition_variable>();
        room_mutexes[room_name] = std::make_shared<std::mutex>();

        send(client_socket, ("Stworzono pokój: " + room_name + "\n").c_str(), room_name.length() + 20, 0);
    } else if (choice == 2) {
        send(client_socket, "Dostępne pokoje:\n", 20, 0);
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (const auto& [room, sockets] : rooms) {
                std::string room_info = room + " (" + std::to_string(sockets.size()) + "/4)\n";
                send(client_socket, room_info.c_str(), room_info.length(), 0);
            }
        }

        send(client_socket, "Podaj nazwę pokoju, do którego chcesz dołączyć: ", 55, 0);
        len = recv(client_socket, buffer, 1024, 0);
        if (len <= 0) {
            close(client_socket);
            return;
        }
        buffer[len] = '\0';
        room_name = buffer;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (rooms.find(room_name) != rooms.end() && rooms[room_name].size() < MAX_CLIENTS) {
                rooms[room_name].insert(client_socket);
                auto game_state = room_states[room_name];
                game_state->guessed_words[client_socket] = std::string(game_state->secret_word.size(), '_');
                game_state->wrong_counts[client_socket] = 0;
                send(client_socket, ("Dołączono do pokoju: " + room_name + "\n").c_str(), room_name.length() + 30, 0);

                send_game_state_to_client(room_name, client_socket);

                if (rooms[room_name].size() == 2) {
                    room_conditions[room_name]->notify_all();
                }
            } else {
                send(client_socket, "Nie można dołączyć do pokoju. Spróbuj ponownie.\n", 59, 0);
                close(client_socket);
                return;
            }
        }
    } else {
        send(client_socket, "Nieprawidłowy wybór. Rozłączanie...\n", 47, 0);
        close(client_socket);
        return;
    }

    broadcast_to_room(room_name, client_nick + " dołączył do pokoju.\n", client_socket);

    // Oczekiwanie na start gry
    {
        auto room_lock = std::unique_lock<std::mutex>(*room_mutexes[room_name]);
        room_conditions[room_name]->wait(room_lock, [&]() {
            return rooms[room_name].size() == 2;
        });
    }

    broadcast_to_room(room_name, "Gra rozpoczyna się!\n");

    while (true) {
        // Pobierz stan pokoju
        auto game_state = room_states[room_name];
        auto& guessed_word = game_state->guessed_words[client_socket];
        auto& wrong_guesses = game_state->wrong_guesses[client_socket];
        auto& wrong_count = game_state->wrong_counts[client_socket];

        while (true) {
            if (game_state->game_over) {
                break;
            }

            // Wyślij stan gry
            send_game_state_to_client(room_name, client_socket);

            // Pobierz literę od gracza
            send(client_socket, "Podaj literę: ", 14, 0);
            int len = recv(client_socket, buffer, 1024, 0);
            if (len <= 0) return;

            buffer[len] = '\0';
            char guessed_char = buffer[0];

            // Sprawdź literę
            {
                std::lock_guard<std::mutex> lock(game_state->game_mutex);
                bool correct_guess = false;
                for (size_t i = 0; i < game_state->secret_word.size(); ++i) {
                    if (game_state->secret_word[i] == guessed_char) {
                        guessed_word[i] = guessed_char;
                        correct_guess = true;
                    }
                }

                if (!correct_guess) {
                    wrong_guesses += guessed_char;
                    wrong_guesses += ' ';
                    wrong_count++;
                }
            }

            // Sprawdź warunki zakończenia gry
            if (guessed_word == game_state->secret_word) {
                send(client_socket, "Gratulacje! Wygrałeś!\n", 26, 0);
                broadcast_to_room(room_name, "Gra zakończona! Wygrał gracz: " + client_nick + "\n", client_socket);

                {
                    std::lock_guard<std::mutex> lock(game_state->game_mutex);
                    game_state->game_over = true;
                }

                std::this_thread::sleep_for(std::chrono::seconds(5));
                {
                    std::lock_guard<std::mutex> lock(*room_mutexes[room_name]);
                    if (rooms[room_name].size() >= 2) {
                        restart_game(room_name);
                    }
                }
                break;

            }

            if (wrong_count >= MAX_WRONG_GUESSES) {
                send(client_socket, ("Przegrałeś! Hasło to: " + game_state->secret_word + "\n").c_str(), game_state->secret_word.size() + 20, 0);
                break;
            }
        }
    }

    // Usuń klienta z pokoju
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        rooms[room_name].erase(client_socket);
        if (rooms[room_name].empty()) {
            rooms.erase(room_name);
            room_states.erase(room_name);
        }
        clients.erase(client_socket);
    }

    broadcast_to_room(room_name, client_nick + " opuścił pokój.\n", client_socket);
    close(client_socket);
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Nie można utworzyć gniazda." << std::endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Nie można ustawić opcji socketu." << std::endl;
        close(server_socket);
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