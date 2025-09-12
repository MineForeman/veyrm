#include "game_screen.h"
#include "input_handler.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

using namespace ftxui;

GameScreen::GameScreen(GameManager* manager, ScreenInteractive*) 
    : game_manager(manager) {}

Component GameScreen::CreateMapPanel() {
    return Renderer([this] {
        // Temporary test map display
        std::vector<Element> map_lines;
        
        // Create a simple test room
        for (int y = 0; y < 20; y++) {
            std::string line;
            for (int x = 0; x < 60; x++) {
                if (y == 0 || y == 19 || x == 0 || x == 59) {
                    line += "#";  // Walls
                } else if (x == game_manager->player_x && y == game_manager->player_y) {
                    line += "@";  // Player at dynamic position
                } else {
                    line += ".";  // Floor
                }
            }
            map_lines.push_back(text(line));
        }
        
        return vbox(map_lines) | border | size(WIDTH, EQUAL, 62) | size(HEIGHT, EQUAL, 22);
    });
}

Component GameScreen::CreateLogPanel() {
    return Renderer([] {
        return vbox({
            text("Welcome to Veyrm!"),
            text("You descend into the Spiral Vaults..."),
            text(""),
            text(""),
            text("Press ? for help, q to quit")
        }) | border | size(HEIGHT, EQUAL, 7);
    });
}

Component GameScreen::CreateStatusPanel() {
    return Renderer([this] {
        return hbox({
            text("HP: " + std::to_string(game_manager->player_hp) + "/" + 
                 std::to_string(game_manager->player_max_hp)) | bold,
            separator(),
            text("Turn: " + std::to_string(game_manager->turn_count)),
            separator(),
            text("Depth: 1"),
        }) | border | size(HEIGHT, EQUAL, 3);
    });
}

Component GameScreen::Create() {
    auto map_panel = CreateMapPanel();
    auto log_panel = CreateLogPanel();
    auto status_panel = CreateStatusPanel();
    
    // Main game layout
    auto layout = Container::Vertical({
        Container::Horizontal({
            map_panel,
            Container::Vertical({
                status_panel,
                log_panel
            })
        })
    });
    
    // Add input handling
    layout |= CatchEvent([this](Event event) {
        InputHandler* input = game_manager->getInputHandler();
        InputAction action = input->processEvent(event);
        
        switch(action) {
            case InputAction::QUIT:
                game_manager->setState(GameState::MENU);
                return true;
                
            case InputAction::MOVE_UP:
                if (game_manager->player_y > 1) {
                    game_manager->player_y--;
                    game_manager->incrementTurn();
                }
                return true;
                
            case InputAction::MOVE_DOWN:
                if (game_manager->player_y < 18) {
                    game_manager->player_y++;
                    game_manager->incrementTurn();
                }
                return true;
                
            case InputAction::MOVE_LEFT:
                if (game_manager->player_x > 1) {
                    game_manager->player_x--;
                    game_manager->incrementTurn();
                }
                return true;
                
            case InputAction::MOVE_RIGHT:
                if (game_manager->player_x < 58) {
                    game_manager->player_x++;
                    game_manager->incrementTurn();
                }
                return true;
                
            case InputAction::WAIT:
                game_manager->incrementTurn();
                return true;
                
            case InputAction::OPEN_INVENTORY:
                game_manager->setState(GameState::INVENTORY);
                return true;
                
            case InputAction::OPEN_HELP:
                game_manager->setState(GameState::HELP);
                return true;
                
            default:
                break;
        }
        
        return false;
    });
    
    return Renderer(layout, [=] {
        return hbox({
            map_panel->Render(),
            vbox({
                status_panel->Render(),
                log_panel->Render()
            })
        });
    });
}