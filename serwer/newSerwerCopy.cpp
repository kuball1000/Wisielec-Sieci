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
const int ROUND_TIME_LIMIT = 240;

std::mutex clients_mutex;
std::unordered_map<int, std::string> clients;
std::unordered_map<std::string, std::unordered_set<int>> rooms;
std::vector<std::string> word_pool = {"example", "network", "hangman", "server", "client"};

// Struktura przechowująca wspólne hasło i stany graczy
class GameState {
public:
    std::string secret_word;
    std::unordered_map<int, std::string> guessed_words;
    std::unordered_map<int, std::string> wrong_guesses;
    std::unordered_map<int, int> wrong_counts;
    std::mutex game_mutex;
    bool game_over = true;
    int winner_socket = -1;

    GameState(const std::string& word) : secret_word(word) {}

    void reset(const std::string& new_word) {
        // std::lock_guard<std::mutex> lock(game_mutex);
        secret_word = new_word;
        game_over = false;
        guessed_words.clear();
        wrong_guesses.clear();
        wrong_counts.clear();
        winner_socket = -1;
    }
};

class Room {
public:
    std::string name;
    std::shared_ptr<GameState> game_state;  // Wspólne hasło
    std::unordered_set<int> clients;       // Gracze w pokoju
    std::unordered_map<int, std::string>& clients_nicks;
    std::mutex room_mutex;
    std::condition_variable room_condition;
    std::unordered_set<int> losers; // Zbiór przegranych graczy
    int owner_socket; // Socket założyciela

    int players_lost = 0;
    bool time_up = false;

    Room(const std::string& room_name, const std::string& initial_word, int owner, std::unordered_map<int, std::string>& nicks)
        : name(room_name), game_state(std::make_shared<GameState>(initial_word)), clients_nicks(nicks), owner_socket(owner) {
        game_state->secret_word = initial_word; // Wspólne hasło
    }

    void broadcast(const std::string& message, int exclude_socket = -1) {
        std::lock_guard<std::mutex> lock(room_mutex);
        for (int socket : clients) {
            if (socket != exclude_socket) {
                send_message(socket, message);
            }
        }
    }

    void reset_game(const std::vector<std::string>& word_pool) {
        std::string new_word = word_pool[rand() % word_pool.size()];
        {
            std::lock_guard<std::mutex> lock(game_state->game_mutex);
            game_state->reset(new_word);
        }
        broadcast("Nowa gra rozpoczyna się! Hasło: " + std::string(game_state->secret_word.size(), '_') + "\n");
        for (int socket : clients) {
            initialize_player_state(socket);
        }
        room_condition.notify_all();
    }

    void send_game_state(int client_socket) {
        auto& guessed_word = game_state->guessed_words[client_socket];
        auto& wrong_guesses = game_state->wrong_guesses[client_socket];
        auto& wrong_count = game_state->wrong_counts[client_socket];

        std::string game_state_msg = "Hasło: " + guessed_word + "\n";
        game_state_msg += "Niepoprawne litery: " + wrong_guesses + "\n";
        game_state_msg += "Pozostałe próby: " + std::to_string(MAX_WRONG_GUESSES - wrong_count) + "\n";
        send_message(client_socket, game_state_msg);
    }

    void start_game() {
    {
        std::unique_lock<std::mutex> lock(room_mutex);
        // Czekaj, aż będzie co najmniej dwóch graczy w pokoju
        room_condition.wait(lock, [&]() { return clients.size() >= 2; });
    }

    std::string command;
    do {
        if (!recv_message(owner_socket, command)) {
            broadcast("Założyciel pokoju rozłączył się. Pokój zostaje zamknięty.\n");
            close_room();
            return;
        }
    } while (command != "/start");
    game_state->game_over = false;


    broadcast("Gra rozpoczyna się!\n");


    for (int client_socket : clients) {
        initialize_player_state(client_socket);
        // std::thread(&Room::handle_client_game, this, client_socket).detach();
    }

    std::thread(&Room::monitor_game_conditions, this).detach();

    reset_game_for_room();

}

void handle_client_game(int client_socket) {
    while (true) {

        if (client_socket == owner_socket && clients.find(client_socket) == clients.end()) {
            broadcast("Założyciel pokoju opuścił grę. Pokój zostaje zamknięty.\n");
            close_room();
            return;
        }

        while (!game_state->game_over && losers.find(client_socket) == losers.end()) { // Pętla dla bieżącej rundy
            send_game_state(client_socket);

            if (!send_message(client_socket, "Podaj literę: ")) {
                {
                    std::lock_guard<std::mutex> lock(room_mutex);
                    clients.erase(client_socket);
                }
                return; // Klient rozłączył się
            }

            std::string guessed_char;
            if (!recv_message(client_socket, guessed_char) || guessed_char.empty()) {
                {
                    std::lock_guard<std::mutex> lock(room_mutex);
                    clients.erase(client_socket);
                }
                return; // Klient rozłączył się
            }

            char letter = guessed_char[0];
            bool correct_guess = false;

            {
                std::lock_guard<std::mutex> lock(game_state->game_mutex);
                for (size_t i = 0; i < game_state->secret_word.size(); ++i) {
                    if (game_state->secret_word[i] == letter) {
                        game_state->guessed_words[client_socket][i] = letter;
                        correct_guess = true;
                    }
                }

                if (!correct_guess) {
                    game_state->wrong_guesses[client_socket] += letter;
                    game_state->wrong_counts[client_socket]++;
                }
            }

            if (game_state->guessed_words[client_socket] == game_state->secret_word) {
                broadcast("Gracz " + clients_nicks[client_socket] + " odgadł hasło: " + game_state->secret_word + "!\n");
                end_round(client_socket, "Odgadłeś hasło! Wygrałeś.\n");
                break;
            }


            if (game_state->wrong_counts[client_socket] == MAX_WRONG_GUESSES) {
                send_message(client_socket, "Przegrałeś! Hasło to: " + game_state->secret_word + "\n");
                {
                    std::lock_guard<std::mutex> lock(room_mutex);
                    losers.insert(client_socket); // Dodanie gracza do zbioru przegranych
                }
                {
                    std::lock_guard<std::mutex> lock(game_state->game_mutex);
                    players_lost++;
                }
                std::cout << "losty: "<< players_lost << std::endl;
                if (players_lost == static_cast<int>(clients.size()) - 1) {
                    int winner = find_last_player();
                    end_round(winner, "Zostałeś ostatnim graczem przy życiu! Wygrywasz.\n");
                    break;
                }
            }
            std::cout << "klienci: : "<< clients.size() << std::endl;
            broadcast_player_status();

        }

        // Poczekaj na nową grę
        {
            std::unique_lock<std::mutex> lock(room_mutex);
            room_condition.wait(lock, [&]() { return !game_state->game_over; });
        }


    }
}

void broadcast_player_list() {
    std::string player_list = "Gracze w pokoju:\n";
    for (int socket : clients) {
        player_list += clients_nicks[socket] + "\n";
    }
    broadcast(player_list);
}


void close_room() {
    std::lock_guard<std::mutex> lock(room_mutex);
    for (int socket : clients) {
        send_message(socket, "Pokój zostaje zamknięty.\n");
        close(socket);
    }
    clients.clear();

    // Usuń pokój z serwera
    // std::lock_guard<std::mutex> server_lock(server_mutex);
    rooms.erase(name);
}


    void monitor_game_conditions() {
        auto start_time = std::chrono::steady_clock::now();
        while (!game_state->game_over) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            {
                std::lock_guard<std::mutex> lock(room_mutex);
                if (clients.size() == 1) {
                    int last_player = *clients.begin();
                    end_round(last_player, "Zostałeś ostatnim graczem w pokoju. Wygrywasz.\n");
                    return;
                }
            }

            auto elapsed_time = std::chrono::duration_cast<std::chrono::minutes>(
                std::chrono::steady_clock::now() - start_time
            );
            if (elapsed_time.count() >= 1) {
                // std::lock_guard<std::mutex> lock(game_state->game_mutex);
                time_up = true;
                broadcast("Czas się skończył! Gra zakończona.\n");
                reset_game_for_room();
                return;
            }
        }
    }


    int find_last_player() {
        std::lock_guard<std::mutex> lock(room_mutex);
        for (int client_socket : clients) {
            if (game_state->wrong_counts[client_socket] < MAX_WRONG_GUESSES) {
                return client_socket;
            }
        }
        return -1; // W razie błędu
    }

void end_round(int winner_socket, const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(game_state->game_mutex);
        if (!game_state->game_over) { // Upewniamy się, że ustawiamy tylko raz
            game_state->game_over = true;
            if (winner_socket != -1) {
                send_message(winner_socket, message);
            }
            broadcast("Runda zakończona! Nowa runda rozpocznie się za 5 sekund...\n");        }
    }

    // Opóźnienie przed rozpoczęciem nowej gry
    std::this_thread::sleep_for(std::chrono::seconds(5));

    reset_game_for_room();
}

void reset_game_for_room() {
    std::string new_word = word_pool[rand() % word_pool.size()];

    {
        std::lock_guard<std::mutex> lock(game_state->game_mutex);
        game_state->reset(new_word);
        game_state->game_over = false; // Resetujemy flagę game_over przed rozpoczęciem nowej rundy
        players_lost = 0;
        time_up = false;
        losers.clear();
    }

    broadcast("Nowa gra rozpoczyna się! Hasło: " + std::string(game_state->secret_word.size(), '_') + "\n");

    for (int client_socket : clients) {
        initialize_player_state(client_socket);
        send_game_state(client_socket); // Wyślij pełny stan gry dla każdego gracza

    }

    // Powiadomienie wątków klientów, że nowa runda się rozpoczęła
    room_condition.notify_all();
}

void broadcast_player_status() {
    std::string status_list = "Stan graczy w pokoju:\n[";
    bool first = true;

    {
        std::lock_guard<std::mutex> lock(game_state->game_mutex);
        for (int socket : clients) {
            if (!first) {
                status_list += ", ";
            }
            status_list += "(" + clients_nicks[socket] + ", " + std::to_string(game_state->wrong_counts[socket]) + ")";
            first = false;
        }
    }

    status_list += "]\n";
    broadcast(status_list);
}

void initialize_player_state(int client_socket) {
    std::lock_guard<std::mutex> lock(game_state->game_mutex);
    game_state->guessed_words[client_socket] = std::string(game_state->secret_word.size(), '_');
    game_state->wrong_guesses[client_socket].clear();
    game_state->wrong_counts[client_socket] = 0;
}


private:
    bool send_message(int socket, const std::string& message) {
        uint32_t len = htonl(message.size());
        if (send(socket, &len, sizeof(len), 0) <= 0) return false;
        if (send(socket, message.c_str(), message.size(), 0) <= 0) return false;
        return true;
    }

    bool recv_message(int socket, std::string& message) {
        uint32_t len = 0;
        if (recv(socket, &len, sizeof(len), MSG_WAITALL) <= 0) return false;
        len = ntohl(len);

        char buffer[1024];
        message.clear();
        while (len > 0) {
            int received = recv(socket, buffer, std::min(len, static_cast<uint32_t>(sizeof(buffer))), 0);
            if (received <= 0) return false;
            message.append(buffer, received);
            len -= received;
        }
        return true;
    }
};

class Server {
private:
    int server_socket;
    std::unordered_map<int, std::string> client_nicks;
    std::unordered_map<std::string, std::shared_ptr<Room>> rooms;
    std::mutex server_mutex;

public:
    Server() {
        srand(static_cast<unsigned>(time(0)));
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1) {
            throw std::runtime_error("Nie można utworzyć gniazda.");
        }

        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            close(server_socket);
            throw std::runtime_error("Nie można ustawić opcji socketu.");
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            close(server_socket);
            throw std::runtime_error("Nie można związać gniazda z adresem.");
        }

        if (listen(server_socket, MAX_CLIENTS) == -1) {
            close(server_socket);
            throw std::runtime_error("Błąd nasłuchiwania.");
        }

        std::cout << "Serwer uruchomiony na porcie " << PORT << std::endl;
    }

    ~Server() {
        close(server_socket);
    }

    void start() {
        while (true) {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

            if (client_socket == -1) {
                std::cerr << "Błąd akceptacji połączenia." << std::endl;
                continue;
            }
            // TODO ADAM
            std::thread(&Server::handle_client, this, client_socket).detach();
        }
    }

void handle_client(int client_socket) {
    // Obsługa wpisywania nicku
    if (!handle_client_nick(client_socket)) {
        return; // Jeśli nie udało się poprawnie ustawić nicku, kończymy
    }

    // Obsługa wyboru dotyczącego pokoju
    handle_room_choice(client_socket);
}

bool handle_client_nick(int client_socket) {
    std::string client_nick;

    if (!send_message(client_socket, "Podaj swój nick: ")) {
        close(client_socket);
        return false;
    }

    if (!recv_message(client_socket, client_nick)) {
        close(client_socket);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(server_mutex);
        bool nick_exists = false;
        for (const auto& pair : client_nicks) {
            if (pair.second == client_nick) {
                nick_exists = true;
                break;
            }
        }

        if (nick_exists) {
            send_message(client_socket, "Nick jest już zajęty. Rozłączanie...");
            close(client_socket);
            return false;
        }

        client_nicks[client_socket] = client_nick;
    }

    std::cout << client_nick << " dołączył do serwera." << std::endl; // do wywalenia
    return true;
}

void handle_room_choice(int client_socket) {
    if (!send_message(client_socket, "Witaj! Możesz stworzyć pokój (1) lub dołączyć do istniejącego (2): ")) {
        close(client_socket);
        return;
    }

    std::string choice_str;
    if (!recv_message(client_socket, choice_str)) {
        close(client_socket);
        return;
    }

    int choice = choice_str[0] - '0';
    if (choice == 1) {
        create_room(client_socket);
    } else if (choice == 2) {
        join_room(client_socket);
    } else {
        send_message(client_socket, "Nieprawidłowy wybór. Rozłączanie...");
        close(client_socket);
    }
}


    void create_room(int client_socket) {
        std::string room_name;
        if (!send_message(client_socket, "Podaj nazwę pokoju: ")) {
            close(client_socket);
            return;
        }

        if (!recv_message(client_socket, room_name)) {
            close(client_socket);
            return;
        }

        std::string initial_word = word_pool[rand() % word_pool.size()];
        auto room = std::make_shared<Room>(room_name, initial_word, client_socket, client_nicks);

        {
            std::lock_guard<std::mutex> lock(server_mutex);
            rooms[room_name] = room;
            room->clients.insert(client_socket);
            std::thread(&Room::handle_client_game, room, client_socket).detach();

        }

        send_message(client_socket, "Stworzono pokój: " + room_name + "\n");
        room->broadcast_player_list(); // Wyświetlenie listy graczy w pokoju
        std::thread(&Room::start_game, room).detach();
    }

    void join_room(int client_socket) {
        if (!send_message(client_socket, "Dostępne pokoje:\n")) {
            close(client_socket);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(server_mutex);
            for (const auto& [room_name, room] : rooms) {
                std::string room_info = room_name + " (" + std::to_string(room->clients.size()) + "/4)\n";
                send_message(client_socket, room_info);
            }
        }

        if (!send_message(client_socket, "Podaj nazwę pokoju, do którego chcesz dołączyć: ")) {
            close(client_socket);
            return;
        }

        std::string room_name;
        if (!recv_message(client_socket, room_name)) {
            close(client_socket);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(server_mutex);
            auto it = rooms.find(room_name);
            if (it != rooms.end() && it->second->clients.size() < MAX_CLIENTS) {
                auto room = it->second;
                room->clients.insert(client_socket);
                std::thread(&Room::handle_client_game, room, client_socket).detach();

                // room->send_game_state(client_socket);
                room->broadcast(client_nicks[client_socket] + " dołączył do pokoju.\n");
                room->broadcast_player_list(); // Wyświetlenie listy graczy w pokoju


                {
                    std::lock_guard<std::mutex> room_lock(room->room_mutex);
                    if (room->clients.size() == 2) {
                        room->room_condition.notify_all();
                    }
                }
            } else {
                send_message(client_socket, "Nie można dołączyć do pokoju. Spróbuj ponownie. (Rozłączanie ...)\n");
                close(client_socket);
                // Dodanie ponownej próby dołaczenia.
            }
        }
    }

private:
    bool send_message(int socket, const std::string& message) {
        uint32_t len = htonl(message.size());
        if (send(socket, &len, sizeof(len), 0) <= 0) return false;
        if (send(socket, message.c_str(), message.size(), 0) <= 0) return false;
        return true;
    }

    bool recv_message(int socket, std::string& message) {
        uint32_t len = 0;
        if (recv(socket, &len, sizeof(len), MSG_WAITALL) <= 0) return false;
        len = ntohl(len);

        char buffer[1024];
        message.clear();
        while (len > 0) {
            int received = recv(socket, buffer, std::min(len, static_cast<uint32_t>(sizeof(buffer))), 0);
            if (received <= 0) return false;
            message.append(buffer, received);
            len -= received;
        }
        return true;
    }
};


int main() {
    try {
        Server server;
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Błąd: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}