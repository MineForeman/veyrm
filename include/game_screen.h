#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "game_state.h"
#include "input_handler.h"
#include <memory>

class MapRenderer;
class StatusBar;
class LayoutSystem;
class InventoryRenderer;

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
    std::unique_ptr<InventoryRenderer> inventory_renderer;
    
    // UI components
    ftxui::Component CreateMapPanel();
    ftxui::Component CreateLogPanel();
    ftxui::Component CreateStatusPanel();
    ftxui::Component CreateInventoryPanel();
    
    // Update layout based on terminal size
    void updateLayout();

    // Handle player movement and attacks
    bool handlePlayerMovement(int dx, int dy, const std::string& direction);

    // Handle door interactions
    bool handleDoorInteraction();

    // Handle inventory input
    bool handleInventoryInput(InputAction action, const ftxui::Event& event);
};