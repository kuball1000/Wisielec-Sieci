#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <SFML/Network.hpp>
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

    // Odbierz długość wiadomości
    if (tcp_socket.receive(&len, sizeof(len), received) != sf::Socket::Done || received != sizeof(len)) {
        return false;
    }
    len = ntohl(len);

    // Odbierz faktyczną wiadomość
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
            std::cout << "Serwer: " << message << std::endl;
        } else {
            std::cerr << "Utracono połączenie z serwerem." << std::endl;
            running = false;
            break;
        }
    }
}

int main() {
    if (tcp_socket.connect(SERVER_IP, SERVER_PORT) != sf::Socket::Done) {
        std::cerr << "Nie można połączyć z serwerem." << std::endl;
        return -1;
    }

    std::cout << "Połączono z serwerem na " << SERVER_IP << ":" << SERVER_PORT << std::endl;

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
        if (!send_message(input_buffer)) {
            std::cerr << "Błąd wysyłania danych." << std::endl;
            running = false;
        }
    }

    return 0;
}
