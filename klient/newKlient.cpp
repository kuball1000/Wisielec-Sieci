#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <arpa/inet.h>
#include <algorithm> // std::find_if
#include <cctype>    // std::isspace

std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    return (start < end) ? std::string(start, end) : std::string();
}


const std::string SERVER_IP = "127.0.0.1";
const unsigned short SERVER_PORT = 12345;

sf::TcpSocket tcp_socket;
std::mutex socket_mutex;
bool running = true;

enum ClientState {
    ENTER_NICK,
    CHOOSE_ACTION,
    ENTER_ROOM_NAME,
    IN_GAME
};

struct GameState {
    std::string guessed_word;
    std::string wrong_letters;
    int remaining_attempts = 0;
    std::unordered_map<std::string, int> opponent_errors; // Nick -> Liczba bledow
};

bool send_message(const std::string& message) {
    std::string sanitized_message = trim(message);
    uint32_t len = htonl(sanitized_message.size());
    if (tcp_socket.send(&len, sizeof(len)) != sf::Socket::Done) {
        return false;
    }
    if (tcp_socket.send(sanitized_message.c_str(), sanitized_message.size()) != sf::Socket::Done) {
        return false;
    }
    return true;
}

bool recv_message(std::string& message) {
    uint32_t len = 0;
    std::size_t received = 0;
    if (tcp_socket.receive(&len, sizeof(len), received) != sf::Socket::Done || received != sizeof(len)) {
        return false;
    }
    len = ntohl(len);

    char buffer[1024];
    message.clear();
    while (len > 0) {
        std::size_t bytes_received = 0;
        if (tcp_socket.receive(buffer, std::min(len, static_cast<uint32_t>(sizeof(buffer))), bytes_received) != sf::Socket::Done) {
            return false;
        }
        message.append(buffer, bytes_received);
        len -= bytes_received;
    }
    return true;
}

void receive_messages(GameState& game_state, ClientState& state) {
    std::string message;
    while (running) {
        if (recv_message(message)) {
            std::cout << "Serwer: " << message << std::endl;

            // Obsługa odpowiedzi serwera
            if (state == CHOOSE_ACTION && message.find("Podaj nazwę pokoju") != std::string::npos) {
                state = ENTER_ROOM_NAME;
            } else if (state == IN_GAME) {
                if (message.find("Hasło:") != std::string::npos) {
                    game_state.guessed_word = message.substr(7);
                } else if (message.find("Niepoprawne litery:") != std::string::npos) {
                    game_state.wrong_letters = message.substr(20);
                } else if (message.find("Pozostałe próby:") != std::string::npos) {
                    game_state.remaining_attempts = std::stoi(message.substr(18));
                } else if (message.find("Błąd gracza") != std::string::npos) {
                    size_t colon_pos = message.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string nick = message.substr(11, colon_pos - 11);
                        int errors = std::stoi(message.substr(colon_pos + 1));
                        game_state.opponent_errors[nick] = errors;
                    }
                }
            }
        } else {
            running = false;
            break;
        }
    }
}

int main() {
    if (tcp_socket.connect(SERVER_IP, SERVER_PORT) != sf::Socket::Done) {
        std::cerr << "Nie mozna polaczyc z serwerem." << std::endl;
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode(800, 600), "Gra Hangman - Klient");
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/arial.ttf")) {
        std::cerr << "Nie mozna zaladowac czcionki." << std::endl;
        return -1;
    }

    ClientState state = ENTER_NICK;
    GameState game_state;

    std::string user_input;
    std::thread(receive_messages, std::ref(game_state), std::ref(state)).detach();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                running = false;
            }
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    if (event.text.unicode == '\b' && !user_input.empty()) {
                        user_input.pop_back();
                    } else if (event.text.unicode != '\b') {
                        user_input += static_cast<char>(event.text.unicode);
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                if (state == ENTER_NICK) {
                    send_message(user_input);
                    std::cout << "Wyslano nick: " << user_input << std::endl;
                    user_input.clear();
                    state = CHOOSE_ACTION;
                } else if (state == CHOOSE_ACTION) {
                    std::string trimmed_input = trim(user_input);
                    if (trimmed_input == "1" || trimmed_input == "2") {
                        send_message(trimmed_input);
                        std::cout << "Wyslano opcje: " << trimmed_input << std::endl;
                        if (trimmed_input == "1") {
                            std::cout << "Oczekiwanie na podanie nazwy pokoju..." << std::endl;
                            state = ENTER_ROOM_NAME;
                        } else if (trimmed_input == "2") {
                            std::cout << "Oczekiwanie na liste pokoi..." << std::endl;
                            state = IN_GAME;
                        }
                    } else {
                        std::cout << "Nieprawidlowy wybor. Sprobuj ponownie.  " << trimmed_input << std::endl;
                    }
                    user_input.clear();
                } else if (state == ENTER_ROOM_NAME) {
                    send_message(user_input);
                    std::cout << "Wyslano nazwe pokoju: " << user_input << std::endl;
                    user_input.clear();
                    state = IN_GAME;
                } else if (state == IN_GAME) {
                    send_message(user_input);
                    std::cout << "Wyslano litere: " << user_input << std::endl;
                    user_input.clear();
                }
            }
        }

        window.clear(sf::Color::White);

        sf::Text text("", font, 20);
        text.setFillColor(sf::Color::Black);

        if (state == ENTER_NICK) {
            text.setString("Podaj swoj nick: " + user_input);
        } else if (state == CHOOSE_ACTION) {
            text.setString("1. Stworz pokoj\n2. Dolacz do pokoju\nWybor: " + user_input);
        } else if (state == ENTER_ROOM_NAME) {
            text.setString("Podaj nazwe pokoju: " + user_input);
        } else if (state == IN_GAME) {
            std::string opponent_info = "Bledy przeciwnikow:\n";
            for (const auto& [nick, errors] : game_state.opponent_errors) {
                opponent_info += nick + ": " + std::to_string(errors) + "\n";
            }

            text.setString(
                "Haslo: " + game_state.guessed_word + "\n" +
                "Niepoprawne litery: " + game_state.wrong_letters + "\n" +
                "Pozostale proby: " + std::to_string(game_state.remaining_attempts) + "\n\n" +
                opponent_info + "\nPodaj litere: " + user_input
            );
        }

        window.draw(text);
        window.display();
    }

    return 0;
}
