#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "game_state.h"

class GameScreen {
public:
    GameScreen(GameManager* manager, ftxui::ScreenInteractive* screen);
    ftxui::Component Create();
    
private:
    GameManager* game_manager;
    
    // UI components
    ftxui::Component CreateMapPanel();
    ftxui::Component CreateLogPanel();
    ftxui::Component CreateStatusPanel();
};