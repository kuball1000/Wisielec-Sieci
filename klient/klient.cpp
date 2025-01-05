#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

const std::string SERVER_IP = "127.0.0.1";
const unsigned short SERVER_PORT = 12345;

sf::TcpSocket socket;
std::mutex socket_mutex;
bool running = true;

// Zmienne współdzielone do komunikacji z GUI
std::string server_message;
std::mutex gui_mutex;

void receive_messages() {
    char buffer[1024];
    std::size_t received;
    while (running) {
        if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Done) {
            buffer[received] = '\0';
            std::lock_guard<std::mutex> lock(gui_mutex);
            server_message = buffer; // Zapisujemy wiadomość od serwera
            std::cout << "Serwer: " << buffer << std::endl;
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Klient Wisielec");
    sf::Font font;

    if (!font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/arial.ttf")) {
        std::cerr << "Nie można załadować czcionki." << std::endl;
        return -1;
    }

    // Elementy GUI
    sf::Text title("Wisielec", font, 50);
    title.setPosition(300, 50);
    title.setFillColor(sf::Color::White);

    sf::Text input_prompt("Podaj nazwę użytkownika:", font, 30);
    input_prompt.setPosition(200, 150);
    input_prompt.setFillColor(sf::Color::White);

    sf::Text input_text("", font, 30);
    input_text.setPosition(200, 200);
    input_text.setFillColor(sf::Color::White);

    sf::Text server_response("", font, 25); // Wyświetlanie odpowiedzi serwera
    server_response.setPosition(50, 400);
    server_response.setFillColor(sf::Color::Green);

    std::string input_buffer;

    if (socket.connect(SERVER_IP, SERVER_PORT) != sf::Socket::Done) {
        std::cerr << "Nie można połączyć z serwerem." << std::endl;
        return -1;
    }

    std::thread(receive_messages).detach();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                running = false;
                window.close();
                socket.disconnect();
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 13) { // Enter
                    std::lock_guard<std::mutex> lock(socket_mutex);
                    if (socket.send(input_buffer.c_str(), input_buffer.size()) != sf::Socket::Done) {
                        std::cerr << "Błąd wysyłania danych." << std::endl;
                    }
                    input_buffer.clear();
                } else if (event.text.unicode == 8) { // Backspace
                    if (!input_buffer.empty()) {
                        input_buffer.pop_back();
                    }
                } else {
                    input_buffer += static_cast<char>(event.text.unicode);
                }
                input_text.setString(input_buffer);
            }
        }

        // Aktualizacja GUI na podstawie wiadomości od serwera
        {
            std::lock_guard<std::mutex> lock(gui_mutex);
            if (!server_message.empty()) {
                server_response.setString("Serwer: " + server_message);
                server_message.clear(); // Czyścimy wiadomość po wyświetleniu
            }
        }

        // Renderowanie okna
        window.clear();
        window.draw(title);
        window.draw(input_prompt);
        window.draw(input_text);
        window.draw(server_response);
        window.display();
    }

    return 0;
}
