#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <SFML/Network.hpp>

const std::string SERVER_IP = "127.0.0.1";
const unsigned short SERVER_PORT = 12345;

sf::TcpSocket socket;
std::mutex socket_mutex;
bool running = true;

void receive_messages() {
    char buffer[1024];
    std::size_t received;
    while (running) {
        if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Done) {
            buffer[received] = '\0';
            std::cout << "Serwer: " << buffer << std::endl;
        }
    }
}

int main() {
    if (socket.connect(SERVER_IP, SERVER_PORT) != sf::Socket::Done) {
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
            socket.disconnect();
            break;
        }

        std::lock_guard<std::mutex> lock(socket_mutex);
        if (socket.send(input_buffer.c_str(), input_buffer.size()) != sf::Socket::Done) {
            std::cerr << "Błąd wysyłania danych." << std::endl;
        }
    }

    return 0;
}
