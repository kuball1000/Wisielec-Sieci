#include "Application.h"

Application::Application()
    : window(sf::VideoMode(800, 600), "SFML Application"),
      currentView(ViewState::Login),
      loginView(window),
      choiceView(window),
      gameView(window) {}

void Application::run() {
    while (window.isOpen()) {
        handleEvents();
        render();
    }
}

void Application::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        switch (currentView) {
            case ViewState::Login:
                if (loginView.handleEvent(event)) {
                    currentView = ViewState::Choice1;
                }
                break;
            case ViewState::Choice1:
                if (choiceView.handleEvent(event, currentRoom)) {
                    currentView = currentRoom.empty() ? ViewState::Choice1 : ViewState::Game;
                }
                break;
            case ViewState::Game:
                if (gameView.handleEvent(event)) {
                    currentView = ViewState::Choice1;
                }
                break;
        }
    }
}

void Application::render() {
    window.clear(sf::Color(200, 200, 200));

    switch (currentView) {
        case ViewState::Login:
            loginView.render();
            break;
        case ViewState::Choice1:
            choiceView.render();
            break;
        case ViewState::Game:
            gameView.render(currentRoom);
            break;
    }

    window.display();
}
