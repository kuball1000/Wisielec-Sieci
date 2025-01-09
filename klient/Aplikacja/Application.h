#ifndef APPLICATION_H
#define APPLICATION_H

#include <SFML/Graphics.hpp>
#include "Resources.h"
#include "Widok_Login.h"
#include "Widok_Choice.h"
#include "Widok_Game.h"

class Application {
public:
    Application();
    void run();

private:
    void handleEvents();
    void render();

    sf::RenderWindow window;
    enum class ViewState { Login, Choice1, Game } currentView;

    Widok_Login loginView;
    Widok_Choice choiceView;
    Widok_Game gameView;

    std::string currentRoom;
};

#endif
