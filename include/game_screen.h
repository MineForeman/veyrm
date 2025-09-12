#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "game_state.h"
#include <memory>

class MapRenderer;

class GameScreen {
public:
    GameScreen(GameManager* manager, ftxui::ScreenInteractive* screen);
    ~GameScreen();  // Needed because MapRenderer is incomplete in header
    ftxui::Component Create();
    
private:
    GameManager* game_manager;
    std::unique_ptr<MapRenderer> renderer;
    
    // UI components
    ftxui::Component CreateMapPanel();
    ftxui::Component CreateLogPanel();
    ftxui::Component CreateStatusPanel();
};