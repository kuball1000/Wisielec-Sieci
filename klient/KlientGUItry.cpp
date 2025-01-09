#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <arpa/inet.h>

const std::string SERVER_IP = "127.0.0.1";
const unsigned short SERVER_PORT = 12345;

sf::TcpSocket tcp_socket;
std::mutex socket_mutex;
bool running = true;

bool send_message(const std::string& message) {
    uint32_t len = htonl(message.size());
    if (tcp_socket.send(&len, sizeof(len)) != sf::Socket::Done) {
        return false;
    }
    if (tcp_socket.send(message.c_str(), message.size()) != sf::Socket::Done) {
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

void receive_messages() {
    std::string message;
    while (running) {
        if (recv_message(message)) {
            std::cout << "\nSerwer: " << message << std::endl;
        } else {
            std::cerr << "\nUtracono połączenie z serwerem." << std::endl;
            running = false;
            break;
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Podaj swój nick", sf::Style::Close);
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/arial.ttf")) {
        std::cerr << "Nie można załadować czcionki." << std::endl;
        return -1;
    }

    sf::Text prompt("Podaj swój nick:", font, 24);
    prompt.setPosition(200, 200);
    prompt.setFillColor(sf::Color::White);

    sf::Text input_text("", font, 24);
    input_text.setPosition(200, 250);
    input_text.setFillColor(sf::Color::White);

    std::string nickname;
    bool nickname_entered = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                running = false;
                return 0;
            }
            if (event.type == sf::Event::TextEntered && !nickname_entered) {
                if (event.text.unicode == 13) { // Enter key
                    nickname_entered = true;
                } else if (event.text.unicode == 8 && !nickname.empty()) { // Backspace
                    nickname.pop_back();
                } else if (event.text.unicode < 128 && event.text.unicode > 31) { // Printable characters
                    nickname += static_cast<char>(event.text.unicode);
                }
                input_text.setString(nickname);
            }
        }

        window.clear(sf::Color::Black);
        window.draw(prompt);
        window.draw(input_text);
        window.display();

        if (nickname_entered) {
            break;
        }
    }

    if (tcp_socket.connect(SERVER_IP, SERVER_PORT) != sf::Socket::Done) {
        std::cerr << "Nie można połączyć z serwerem." << std::endl;
        return -1;
    }

    std::cout << "Połączono z serwerem na " << SERVER_IP << ":" << SERVER_PORT << std::endl;

    if (!send_message("[Nick] " + nickname)) {
        std::cerr << "Błąd wysyłania nicku." << std::endl;
        return -1;
    }

    std::thread(receive_messages).detach();

    std::string input_buffer;
    while (running) {
        std::cout << "Wpisz wiadomość: ";
        std::getline(std::cin, input_buffer);

        if (input_buffer == "/exit") {
            running = false;
            tcp_socket.disconnect();
            break;
        }

        std::lock_guard<std::mutex> lock(socket_mutex);
        if (!send_message("[" + nickname + "] " + input_buffer)) {
            std::cerr << "Błąd wysyłania danych." << std::endl;
            running = false;
        }
    }

    return 0;
}
