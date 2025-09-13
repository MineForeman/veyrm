#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "game_state.h"
#include <memory>

class MapRenderer;
class StatusBar;
class LayoutSystem;

class GameScreen {
public:
    GameScreen(GameManager* manager, ftxui::ScreenInteractive* screen);
    ~GameScreen();  // Needed because MapRenderer is incomplete in header
    ftxui::Component Create();
    
private:
    GameManager* game_manager;
    [[maybe_unused]] ftxui::ScreenInteractive* screen_ref;
    std::unique_ptr<MapRenderer> renderer;
    std::unique_ptr<StatusBar> status_bar;
    std::unique_ptr<LayoutSystem> layout_system;
    
    // UI components
    ftxui::Component CreateMapPanel();
    ftxui::Component CreateLogPanel();
    ftxui::Component CreateStatusPanel();
    
    // Update layout based on terminal size
    void updateLayout();
};