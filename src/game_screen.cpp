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
#include "layout_system.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace ftxui;

GameScreen::GameScreen(GameManager* manager, ScreenInteractive* screen) 
    : game_manager(manager),
      screen_ref(screen),
      renderer(std::make_unique<MapRenderer>(200, 60)),  // Large default for fullscreen
      status_bar(std::make_unique<StatusBar>()),
      layout_system(std::make_unique<LayoutSystem>()) {
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
        
        // Get actual terminal dimensions
        auto term_size = Terminal::Size();
        
        // Calculate map viewport size
        // Reserve more space for right panels to ensure no overlap
        // Right panels need at least 40 chars + borders/separators
        int right_panel_width = 45;  // Increased to ensure separation
        int map_width = std::max(50, term_size.dimx - right_panel_width - 4);  // -4 for borders
        int map_height = std::max(20, term_size.dimy - 2);  // -2 for borders
        
        // Clamp map width to prevent it from being too wide
        map_width = std::min(map_width, term_size.dimx - 50);  // Ensure at least 50 chars for right side
        
        // Update renderer viewport to match available space
        renderer->setViewport(map_width, map_height);
        
        // Use the new Renderer class to render the map
        Element rendered = renderer->render(*map, *game_manager);
        
        // Add border and size constraints to prevent overlap
        // Use explicit width to ensure it doesn't expand into right panel area
        return rendered | border | 
               size(WIDTH, EQUAL, map_width + 2);  // +2 for borders
    });
}

Component GameScreen::CreateLogPanel() {
    return Renderer([this] {
        // Fixed width for right panel to ensure consistent layout
        return game_manager->getMessageLog()->render(10) | 
               border | 
               size(WIDTH, EQUAL, 42);  // Fixed width
    });
}

Component GameScreen::CreateStatusPanel() {
    return Renderer([this] {
        // Fixed width matching log panel
        return status_bar->render(*game_manager) | 
               size(WIDTH, EQUAL, 42);  // Fixed width
    });
}

void GameScreen::updateLayout() {
    // Since we're using Fullscreen mode, FTXUI automatically handles the full terminal
    // We don't have direct access to dimensions from ScreenInteractive
    // But we can use terminal size detection
    
    // Get terminal dimensions using FTXUI's terminal utilities
    int width = 80;   // Default fallback
    int height = 24;  // Default fallback
    
    // Try to get actual terminal size
    // FTXUI uses the full screen in Fullscreen mode
    // The renderer will use all available space
    
    // For now, we'll let FTXUI handle the sizing automatically
    // The layout system will adapt to whatever space is available
    layout_system->updateDimensions(width, height);
}

Component GameScreen::Create() {
    auto map_panel = CreateMapPanel();
    auto log_panel = CreateLogPanel();
    auto status_panel = CreateStatusPanel();
    
    // Create structured layout with proper separation
    // The key is to use Container::Horizontal to properly separate left and right
    auto right_panel = Container::Vertical({
        status_panel,
        log_panel
    });
    
    auto layout = Container::Horizontal({
        map_panel,     // Map on the left
        right_panel    // Status and log on the right
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
                        game_manager->updateFOV();
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
                        game_manager->updateFOV();
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
                        game_manager->updateFOV();
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
                        game_manager->updateFOV();
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