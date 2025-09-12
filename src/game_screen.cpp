#include "game_screen.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "renderer.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

using namespace ftxui;

GameScreen::GameScreen(GameManager* manager, ScreenInteractive*) 
    : game_manager(manager),
      renderer(std::make_unique<MapRenderer>(80, 24)) {
    // Center renderer on player's starting position
    renderer->centerOn(game_manager->player_x, game_manager->player_y);
}

GameScreen::~GameScreen() = default;

Component GameScreen::CreateMapPanel() {
    return Renderer([this] {
        auto* map = game_manager->getMap();
        if (!map) {
            return text("No map loaded") | border;
        }
        
        // Use the new Renderer class to render the map
        Element rendered = renderer->render(*map, *game_manager);
        
        // Add border and sizing
        return rendered | border | size(WIDTH, EQUAL, 82) | size(HEIGHT, EQUAL, 26);
    });
}

Component GameScreen::CreateLogPanel() {
    return Renderer([this] {
        return game_manager->getMessageLog()->render(5) | 
               border | 
               size(HEIGHT, EQUAL, 7);
    });
}

Component GameScreen::CreateStatusPanel() {
    return Renderer([this] {
        auto tm = game_manager->getTurnManager();
        std::vector<Element> status_elements = {
            text("HP: " + std::to_string(game_manager->player_hp) + "/" + 
                 std::to_string(game_manager->player_max_hp)) | bold,
            separator(),
            text("Turn: " + std::to_string(tm->getCurrentTurn())),
            separator(),
            text("Time: " + std::to_string(tm->getWorldTime())),
            separator(),
            text("Depth: 1")
        };
        
        // Add FPS counter in debug mode
        if (game_manager->isDebugMode() && game_manager->getFrameStats()) {
            status_elements.push_back(separator());
            status_elements.push_back(text(game_manager->getFrameStats()->format()) | color(Color::Green));
        }
        
        return hbox(status_elements) | border | size(HEIGHT, EQUAL, 3);
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
                
            case InputAction::MOVE_UP: {
                auto* map = game_manager->getMap();
                int new_x = game_manager->player_x;
                int new_y = game_manager->player_y - 1;
                if (map && map->isWalkable(new_x, new_y)) {
                    game_manager->player_y = new_y;
                    game_manager->processPlayerAction(ActionSpeed::NORMAL);
                    game_manager->getMessageLog()->addMessage("You move north.");
                }
                return true;
            }
                
            case InputAction::MOVE_DOWN: {
                auto* map = game_manager->getMap();
                int new_x = game_manager->player_x;
                int new_y = game_manager->player_y + 1;
                if (map && map->isWalkable(new_x, new_y)) {
                    game_manager->player_y = new_y;
                    game_manager->processPlayerAction(ActionSpeed::NORMAL);
                    game_manager->getMessageLog()->addMessage("You move south.");
                }
                return true;
            }
                
            case InputAction::MOVE_LEFT: {
                auto* map = game_manager->getMap();
                int new_x = game_manager->player_x - 1;
                int new_y = game_manager->player_y;
                if (map && map->isWalkable(new_x, new_y)) {
                    game_manager->player_x = new_x;
                    game_manager->processPlayerAction(ActionSpeed::NORMAL);
                    game_manager->getMessageLog()->addMessage("You move west.");
                }
                return true;
            }
                
            case InputAction::MOVE_RIGHT: {
                auto* map = game_manager->getMap();
                int new_x = game_manager->player_x + 1;
                int new_y = game_manager->player_y;
                if (map && map->isWalkable(new_x, new_y)) {
                    game_manager->player_x = new_x;
                    game_manager->processPlayerAction(ActionSpeed::NORMAL);
                    game_manager->getMessageLog()->addMessage("You move east.");
                }
                return true;
            }
                
            case InputAction::WAIT:
                game_manager->processPlayerAction(ActionSpeed::NORMAL);
                game_manager->getMessageLog()->addMessage("You wait.");
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