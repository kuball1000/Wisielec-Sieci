// Serwer w C++ dla gry "Wisielec"
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

const int PORT = 12345;
const int MAX_CLIENTS = 4;
const int MAX_WRONG_GUESSES = 6;

std::mutex clients_mutex;
std::unordered_map<int, std::string> clients;
std::unordered_map<std::string, std::unordered_set<int>> rooms;
std::vector<std::string> word_pool = {"example", "network", "hangman", "server", "client"};

void broadcast_to_room(const std::string& room_name, const std::string& message, int exclude_socket = -1) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int socket : rooms[room_name]) {
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

    std::cout << client_nick << " dołączył do serwera." << std::endl;
    send(client_socket, "Witaj! Możesz stworzyć pokój (1) lub dołączyć do istniejącego (2): ", 58, 0);

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

        send(client_socket, ("Stworzono pokój: " + room_name + "\n").c_str(), room_name.length() + 17, 0);
    } else if (choice == 2) {
        send(client_socket, "Dostępne pokoje:\n", 20, 0);
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (const auto& [room, sockets] : rooms) {
                std::string room_info = room + " (" + std::to_string(sockets.size()) + "/4)\n";
                send(client_socket, room_info.c_str(), room_info.length(), 0);
            }
        }

        send(client_socket, "Podaj nazwę pokoju, do którego chcesz dołączyć: ", 49, 0);
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
                send(client_socket, ("Dołączono do pokoju: " + room_name + "\n").c_str(), room_name.length() + 19, 0);
            } else {
                send(client_socket, "Nie można dołączyć do pokoju. Spróbuj ponownie.\n", 49, 0);
                close(client_socket);
                return;
            }
        }
    } else {
        send(client_socket, "Nieprawidłowy wybór. Rozłączanie...\n", 37, 0);
        close(client_socket);
        return;
    }

    broadcast_to_room(room_name, client_nick + " dołączył do pokoju.\n", client_socket);

    // Rozpoczęcie gry w pokoju
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

    // Usuń klienta z pokoju
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        rooms[room_name].erase(client_socket);
        if (rooms[room_name].empty()) {
            rooms.erase(room_name);
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
