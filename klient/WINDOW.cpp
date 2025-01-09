#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <string>
#include <vector>

class Application {
public:
    Application() : window(sf::VideoMode(800, 600), "SFML Views App") {
        window.setFramerateLimit(60);
        currentView = ViewType::InputView;
        setupInputView();
        setupMainMenuView();
        setupCreateRoomView();
        setupJoinRoomView();
        setupGameView();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            render();
        }
    }

private:
    enum class ViewType {
        InputView,
        MainMenuView,
        CreateRoomView,
        JoinRoomView,
        GameView
    };

    sf::RenderWindow window;
    ViewType currentView;

    // Fonts and shared elements
    sf::Font font;

    // Input View elements
    sf::Text nicknameLabel;
    sf::RectangleShape inputBox;
    sf::Text nicknameText;
    sf::RectangleShape connectButton;
    sf::Text connectButtonText;

    // Main Menu View elements
    sf::Text menuTitle;
    sf::Text createRoomNumber;
    sf::Text joinRoomNumber;
    sf::RectangleShape createRoomButton;
    sf::Text createRoomButtonText;
    sf::RectangleShape joinRoomButton;
    sf::Text joinRoomButtonText;

    // Create Room View elements
    sf::Text createRoomLabel;
    sf::RectangleShape createRoomInputBox;
    sf::Text createRoomInputText;
    sf::RectangleShape confirmCreateRoomButton;
    sf::Text confirmCreateRoomButtonText;
    sf::RectangleShape createRoomBackButton;
    sf::Text createRoomBackButtonText;

    // Join Room View elements
    sf::Text joinRoomLabel;
    std::vector<sf::RectangleShape> roomButtons;
    std::vector<sf::Text> roomButtonTexts;
    sf::RectangleShape joinRoomBackButton;
    sf::Text joinRoomBackButtonText;

    // Game View elements
    sf::Text roomTitle;
    sf::RectangleShape gameBackButton;
    sf::Text gameBackButtonText;

    // Room data
    std::vector<std::string> roomNames;
    std::string nickname;
    std::string roomNameInput;
    std::string currentRoomName;

    void setupInputView() {
        font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/arial.ttf");

        // Nickname label
        nicknameLabel.setFont(font);
        nicknameLabel.setString("Nick:");
        nicknameLabel.setCharacterSize(24);
        nicknameLabel.setPosition(300, 200);
        nicknameLabel.setFillColor(sf::Color::Black);

        // Input box
        inputBox.setSize(sf::Vector2f(200, 40));
        inputBox.setFillColor(sf::Color(230, 230, 230));
        inputBox.setPosition(300, 250);

        // Nickname text
        nicknameText.setFont(font);
        nicknameText.setCharacterSize(20);
        nicknameText.setPosition(310, 260);
        nicknameText.setFillColor(sf::Color::Black);

        // Connect button
        connectButton.setSize(sf::Vector2f(100, 40));
        connectButton.setFillColor(sf::Color(100, 149, 237));
        connectButton.setPosition(350, 320);

        connectButtonText.setFont(font);
        connectButtonText.setString("Polacz");
        connectButtonText.setCharacterSize(20);
        connectButtonText.setFillColor(sf::Color::White);
        connectButtonText.setPosition(365, 325);
    }

    void setupMainMenuView() {
        // Menu title
        menuTitle.setFont(font);
        menuTitle.setString("Wybierz opcje:");
        menuTitle.setCharacterSize(24);
        menuTitle.setPosition(300, 50);
        menuTitle.setFillColor(sf::Color::Black);

        // Create Room option
        createRoomNumber.setFont(font);
        createRoomNumber.setString("1.");
        createRoomNumber.setCharacterSize(20);
        createRoomNumber.setPosition(50, 150);
        createRoomNumber.setFillColor(sf::Color::Black);

        createRoomButton.setSize(sf::Vector2f(200, 40));
        createRoomButton.setFillColor(sf::Color(100, 149, 237));
        createRoomButton.setPosition(100, 150);

        createRoomButtonText.setFont(font);
        createRoomButtonText.setString("Stworz nowy pokoj");
        createRoomButtonText.setCharacterSize(20);
        createRoomButtonText.setFillColor(sf::Color::White);
        createRoomButtonText.setPosition(110, 160);

        // Join Room option
        joinRoomNumber.setFont(font);
        joinRoomNumber.setString("2.");
        joinRoomNumber.setCharacterSize(20);
        joinRoomNumber.setPosition(50, 220);
        joinRoomNumber.setFillColor(sf::Color::Black);

        joinRoomButton.setSize(sf::Vector2f(200, 40));
        joinRoomButton.setFillColor(sf::Color(100, 149, 237));
        joinRoomButton.setPosition(100, 220);

        joinRoomButtonText.setFont(font);
        joinRoomButtonText.setString("Dolacz do pokoju");
        joinRoomButtonText.setCharacterSize(20);
        joinRoomButtonText.setFillColor(sf::Color::White);
        joinRoomButtonText.setPosition(110, 230);
    }

    void setupCreateRoomView() {
        // Create Room label
        createRoomLabel.setFont(font);
        createRoomLabel.setString("Wpisz nazwe pokoju:");
        createRoomLabel.setCharacterSize(24);
        createRoomLabel.setPosition(300, 50);
        createRoomLabel.setFillColor(sf::Color::Black);

        // Input box
        createRoomInputBox.setSize(sf::Vector2f(300, 40));
        createRoomInputBox.setFillColor(sf::Color(230, 230, 230));
        createRoomInputBox.setPosition(250, 150);

        createRoomInputText.setFont(font);
        createRoomInputText.setCharacterSize(20);
        createRoomInputText.setPosition(260, 160);
        createRoomInputText.setFillColor(sf::Color::Black);

        // Confirm button
        confirmCreateRoomButton.setSize(sf::Vector2f(150, 40));
        confirmCreateRoomButton.setFillColor(sf::Color(100, 149, 237));
        confirmCreateRoomButton.setPosition(325, 220);

        confirmCreateRoomButtonText.setFont(font);
        confirmCreateRoomButtonText.setString("Stworz pokoj");
        confirmCreateRoomButtonText.setCharacterSize(20);
        confirmCreateRoomButtonText.setFillColor(sf::Color::White);
        confirmCreateRoomButtonText.setPosition(340, 230);

        // Back button
        createRoomBackButton.setSize(sf::Vector2f(100, 40));
        createRoomBackButton.setFillColor(sf::Color(100, 149, 237));
        createRoomBackButton.setPosition(50, 500);

        createRoomBackButtonText.setFont(font);
        createRoomBackButtonText.setString("Powrot");
        createRoomBackButtonText.setCharacterSize(20);
        createRoomBackButtonText.setFillColor(sf::Color::White);
        createRoomBackButtonText.setPosition(65, 510);
    }

    void setupJoinRoomView() {
        // Join Room label
        joinRoomLabel.setFont(font);
        joinRoomLabel.setString("Wybierz pokoj:");
        joinRoomLabel.setCharacterSize(24);
        joinRoomLabel.setPosition(300, 50);
        joinRoomLabel.setFillColor(sf::Color::Black);

        // Back button
        joinRoomBackButton.setSize(sf::Vector2f(100, 40));
        joinRoomBackButton.setFillColor(sf::Color(100, 149, 237));
        joinRoomBackButton.setPosition(50, 500);

        joinRoomBackButtonText.setFont(font);
        joinRoomBackButtonText.setString("Powrot");
        joinRoomBackButtonText.setCharacterSize(20);
        joinRoomBackButtonText.setFillColor(sf::Color::White);
        joinRoomBackButtonText.setPosition(65, 510);

        setupRoomButtons();
    }

    void setupRoomButtons() {
        roomButtons.clear();
        roomButtonTexts.clear();

        for (size_t i = 0; i < roomNames.size(); ++i) {
            sf::RectangleShape button;
            button.setSize(sf::Vector2f(200, 40));
            button.setFillColor(sf::Color(100, 149, 237));
            button.setPosition(300, 150 + i * 60);

            sf::Text text;
            text.setFont(font);
            text.setString(roomNames[i]);
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::White);
            text.setPosition(310, 160 + i * 60);

            roomButtons.push_back(button);
            roomButtonTexts.push_back(text);
        }
    }

    void setupGameView() {
        // Room title
        roomTitle.setFont(font);
        roomTitle.setCharacterSize(24);
        roomTitle.setPosition(300, 50);
        roomTitle.setFillColor(sf::Color::Black);

        // Back button
        gameBackButton.setSize(sf::Vector2f(100, 40));
        gameBackButton.setFillColor(sf::Color(100, 149, 237));
        gameBackButton.setPosition(50, 500);

        gameBackButtonText.setFont(font);
        gameBackButtonText.setString("Powrot");
        gameBackButtonText.setCharacterSize(20);
        gameBackButtonText.setFillColor(sf::Color::White);
        gameBackButtonText.setPosition(65, 510);
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (currentView == ViewType::InputView) {
                handleInputViewEvents(event);
            } else if (currentView == ViewType::MainMenuView) {
                handleMainMenuViewEvents(event);
            } else if (currentView == ViewType::CreateRoomView) {
                handleCreateRoomViewEvents(event);
            } else if (currentView == ViewType::JoinRoomView) {
                handleJoinRoomViewEvents(event);
            } else if (currentView == ViewType::GameView) {
                handleGameViewEvents(event);
            }
        }
    }

    void handleInputViewEvents(const sf::Event& event) {
        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode == 8 && !nickname.empty()) { // Backspace
                nickname.pop_back();
            } else if (event.text.unicode >= 32 && event.text.unicode <= 126) { // Printable characters
                nickname += static_cast<char>(event.text.unicode);
            }
            nicknameText.setString(nickname);
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                if (connectButton.getGlobalBounds().contains(mousePos)) {
                    currentView = ViewType::MainMenuView;
                }
            }
        }
    }

    void handleMainMenuViewEvents(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            if (createRoomButton.getGlobalBounds().contains(mousePos)) {
                currentView = ViewType::CreateRoomView;
            } else if (joinRoomButton.getGlobalBounds().contains(mousePos)) {
                currentView = ViewType::JoinRoomView;
            }
        }
    }

    void handleCreateRoomViewEvents(const sf::Event& event) {
        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode == 8 && !roomNameInput.empty()) { // Backspace
                roomNameInput.pop_back();
            } else if (event.text.unicode >= 32 && event.text.unicode <= 126) { // Printable characters
                roomNameInput += static_cast<char>(event.text.unicode);
            }
            createRoomInputText.setString(roomNameInput);
        } else if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            if (confirmCreateRoomButton.getGlobalBounds().contains(mousePos)) {
                roomNames.push_back(roomNameInput);
                setupRoomButtons();
                currentRoomName = roomNameInput;
                roomNameInput.clear();
                createRoomInputText.setString("");
                roomTitle.setString(currentRoomName);
                currentView = ViewType::GameView;
            } else if (createRoomBackButton.getGlobalBounds().contains(mousePos)) {
                currentView = ViewType::MainMenuView;
            }
        }
    }

    void handleJoinRoomViewEvents(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            for (size_t i = 0; i < roomButtons.size(); ++i) {
                if (roomButtons[i].getGlobalBounds().contains(mousePos)) {
                    currentRoomName = roomNames[i];
                    roomTitle.setString(currentRoomName);
                    currentView = ViewType::GameView;
                    return;
                }
            }
            if (joinRoomBackButton.getGlobalBounds().contains(mousePos)) {
                currentView = ViewType::MainMenuView;
            }
        }
    }

    void handleGameViewEvents(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            if (gameBackButton.getGlobalBounds().contains(mousePos)) {
                currentView = ViewType::MainMenuView;
            }
        }
    }

    void render() {
        window.clear(sf::Color(245, 245, 245));

        if (currentView == ViewType::InputView) {
            renderInputView();
        } else if (currentView == ViewType::MainMenuView) {
            renderMainMenuView();
        } else if (currentView == ViewType::CreateRoomView) {
            renderCreateRoomView();
        } else if (currentView == ViewType::JoinRoomView) {
            renderJoinRoomView();
        } else if (currentView == ViewType::GameView) {
            renderGameView();
        }

        window.display();
    }

    void renderInputView() {
        window.draw(nicknameLabel);
        window.draw(inputBox);
        window.draw(nicknameText);
        window.draw(connectButton);
        window.draw(connectButtonText);
    }

    void renderMainMenuView() {
        window.draw(menuTitle);
        window.draw(createRoomNumber);
        window.draw(createRoomButton);
        window.draw(createRoomButtonText);
        window.draw(joinRoomNumber);
        window.draw(joinRoomButton);
        window.draw(joinRoomButtonText);
    }

    void renderCreateRoomView() {
        window.draw(createRoomLabel);
        window.draw(createRoomInputBox);
        window.draw(createRoomInputText);
        window.draw(confirmCreateRoomButton);
        window.draw(confirmCreateRoomButtonText);
        window.draw(createRoomBackButton);
        window.draw(createRoomBackButtonText);
    }

    void renderJoinRoomView() {
        window.draw(joinRoomLabel);
        for (size_t i = 0; i < roomButtons.size(); ++i) {
            window.draw(roomButtons[i]);
            window.draw(roomButtonTexts[i]);
        }
        window.draw(joinRoomBackButton);
        window.draw(joinRoomBackButtonText);
    }

    void renderGameView() {
        window.draw(roomTitle);
        window.draw(gameBackButton);
        window.draw(gameBackButtonText);
    }
};

int main() {
    Application app;
    app.run();
    return 0;
}
