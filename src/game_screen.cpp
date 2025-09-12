#include "game_screen.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "renderer.h"
#include "entity_manager.h"
#include "player.h"
#include "status_bar.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

using namespace ftxui;

GameScreen::GameScreen(GameManager* manager, ScreenInteractive*) 
    : game_manager(manager),
      renderer(std::make_unique<MapRenderer>(80, 24)),
      status_bar(std::make_unique<StatusBar>()) {
    // Center renderer on player's starting position
    if (auto* player = game_manager->getPlayer()) {
        renderer->centerOn(player->x, player->y);
    } else {
        // Fallback to deprecated variables if entity system not initialized
        renderer->centerOn(game_manager->player_x, game_manager->player_y);
    }
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
        return status_bar->render(*game_manager);
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
                auto* player = game_manager->getPlayer();
                auto* map = game_manager->getMap();
                auto* entity_manager = game_manager->getEntityManager();
                
                if (player && map && entity_manager) {
                    if (player->tryMove(*map, entity_manager, 0, -1)) {
                        game_manager->processPlayerAction(ActionSpeed::NORMAL);
                        game_manager->getMessageLog()->addMessage("You move north.");
                        renderer->centerOn(player->x, player->y);
                    }
                }
                return true;
            }
                
            case InputAction::MOVE_DOWN: {
                auto* player = game_manager->getPlayer();
                auto* map = game_manager->getMap();
                auto* entity_manager = game_manager->getEntityManager();
                
                if (player && map && entity_manager) {
                    if (player->tryMove(*map, entity_manager, 0, 1)) {
                        game_manager->processPlayerAction(ActionSpeed::NORMAL);
                        game_manager->getMessageLog()->addMessage("You move south.");
                        renderer->centerOn(player->x, player->y);
                    }
                }
                return true;
            }
                
            case InputAction::MOVE_LEFT: {
                auto* player = game_manager->getPlayer();
                auto* map = game_manager->getMap();
                auto* entity_manager = game_manager->getEntityManager();
                
                if (player && map && entity_manager) {
                    if (player->tryMove(*map, entity_manager, -1, 0)) {
                        game_manager->processPlayerAction(ActionSpeed::NORMAL);
                        game_manager->getMessageLog()->addMessage("You move west.");
                        renderer->centerOn(player->x, player->y);
                    }
                }
                return true;
            }
                
            case InputAction::MOVE_RIGHT: {
                auto* player = game_manager->getPlayer();
                auto* map = game_manager->getMap();
                auto* entity_manager = game_manager->getEntityManager();
                
                if (player && map && entity_manager) {
                    if (player->tryMove(*map, entity_manager, 1, 0)) {
                        game_manager->processPlayerAction(ActionSpeed::NORMAL);
                        game_manager->getMessageLog()->addMessage("You move east.");
                        renderer->centerOn(player->x, player->y);
                    }
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