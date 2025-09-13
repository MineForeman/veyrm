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
#include "log.h"
#include "monster.h"
#include "combat_system.h"
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

bool GameScreen::handleDoorInteraction() {
    auto* player = game_manager->getPlayer();
    auto* map = game_manager->getMap();
    auto* msg_log = game_manager->getMessageLog();

    if (!player || !map || !msg_log) {
        return false;
    }

    LOG_PLAYER("Attempting to interact with doors");

    // Check all 8 directions for doors
    const int dirs[8][2] = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0},   // Cardinal
        {-1, -1}, {1, -1}, {-1, 1}, {1, 1}  // Diagonal
    };

    bool found_door = false;
    int door_count = 0;

    for (const auto& dir : dirs) {
        int check_x = player->x + dir[0];
        int check_y = player->y + dir[1];

        if (!map->inBounds(check_x, check_y)) {
            continue;
        }

        TileType tile = map->getTile(check_x, check_y);

        if (tile == TileType::DOOR_CLOSED) {
            // Open the door
            map->setTile(check_x, check_y, TileType::DOOR_OPEN);
            msg_log->addMessage("You open the door.");
            LOG_ENVIRONMENT("Door opened at (" + std::to_string(check_x) + ", " + std::to_string(check_y) + ")");
            LOG_PLAYER("Player opened a closed door");
            found_door = true;
            door_count++;
        } else if (tile == TileType::DOOR_OPEN) {
            // Close the door
            map->setTile(check_x, check_y, TileType::DOOR_CLOSED);
            msg_log->addMessage("You close the door.");
            LOG_ENVIRONMENT("Door closed at (" + std::to_string(check_x) + ", " + std::to_string(check_y) + ")");
            LOG_PLAYER("Player closed an open door");
            found_door = true;
            door_count++;
        }
    }

    if (!found_door) {
        msg_log->addMessage("There is no door here.");
        LOG_PLAYER("Player tried to interact with door but none found");
        return false;
    }

    // Opening/closing doors takes a turn
    game_manager->processPlayerAction(ActionSpeed::NORMAL);
    game_manager->updateMonsters();

    // If multiple doors, add a note
    if (door_count > 1) {
        msg_log->addMessage("(Multiple doors toggled)");
    }

    return true;
}

bool GameScreen::handlePlayerMovement(int dx, int dy, const std::string& direction) {
    auto* player = game_manager->getPlayer();
    auto* map = game_manager->getMap();
    auto* entity_manager = game_manager->getEntityManager();
    auto* combat_system = game_manager->getCombatSystem();

    LOG_PLAYER("Moving " + direction + " (dx=" + std::to_string(dx) + ", dy=" + std::to_string(dy) + ")");

    if (!player || !map || !entity_manager) {
        LOG_ERROR("Missing required components for player movement");
        return false;
    }

    int new_x = player->x + dx;
    int new_y = player->y + dy;
    LOG_PLAYER("Target position: (" + std::to_string(new_x) + ", " + std::to_string(new_y) + ")");

    // Check for monsters to attack
    auto blocking = entity_manager->getBlockingEntityAt(new_x, new_y);
    if (blocking && blocking->is_monster) {
        LOG_COMBAT("Player bumping into monster at (" + std::to_string(new_x) + ", " + std::to_string(new_y) + ")");

        // Attack the monster
        if (combat_system) {
            LOG_COMBAT("Initiating player attack on monster");
            auto result = combat_system->processAttack(*player, *blocking);

            // Log messages to both debug log and game message log
            auto* msg_log = game_manager->getMessageLog();
            if (msg_log) {
                LOG_INFO("Adding combat messages to game message log");
                if (!result.attack_message.empty()) {
                    msg_log->addMessage(result.attack_message);
                    LOG_INFO("Added attack message: " + result.attack_message);
                }
                if (!result.damage_message.empty()) {
                    msg_log->addMessage(result.damage_message);
                    LOG_INFO("Added damage message: " + result.damage_message);
                }
                if (!result.result_message.empty()) {
                    msg_log->addMessage(result.result_message);
                    LOG_INFO("Added result message: " + result.result_message);
                }
            }

            if (result.fatal) {
                LOG_COMBAT("Monster killed! Removing from entity manager");
                entity_manager->destroyEntity(blocking);

                // Award experience if it's a monster
                if (auto* monster = dynamic_cast<Monster*>(blocking.get())) {
                    int exp = monster->xp_value;
                    player->gainExperience(exp);
                    msg_log->addMessage("You gain " + std::to_string(exp) + " experience.");
                    LOG_COMBAT("Player gained " + std::to_string(exp) + " experience");
                }
            }
        } else {
            LOG_ERROR("Combat system not available!");
        }

        game_manager->processPlayerAction(ActionSpeed::NORMAL);
        game_manager->updateMonsters();  // Let monsters respond
        return true;
    } else if (player->tryMove(*map, entity_manager, dx, dy)) {
        LOG_PLAYER("Moved successfully to (" + std::to_string(player->x) + ", " + std::to_string(player->y) + ")");
        game_manager->processPlayerAction(ActionSpeed::NORMAL);

        // Only add movement message if not in combat
        if (!direction.empty()) {
            // Commenting out movement messages to reduce log spam
            // game_manager->getMessageLog()->addMessage("You move " + direction + ".");
        }

        renderer->centerOn(player->x, player->y);
        game_manager->updateFOV();
        game_manager->updateMonsters();  // Let monsters act after player moves
        return true;
    } else {
        LOG_PLAYER("Movement blocked");
    }
    return false;
}

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
                
            case InputAction::MOVE_UP:
                return handlePlayerMovement(0, -1, "north");

            case InputAction::MOVE_DOWN:
                return handlePlayerMovement(0, 1, "south");

            case InputAction::MOVE_LEFT:
                return handlePlayerMovement(-1, 0, "west");

            case InputAction::MOVE_RIGHT:
                return handlePlayerMovement(1, 0, "east");

            case InputAction::MOVE_UP_LEFT:
                return handlePlayerMovement(-1, -1, "northwest");

            case InputAction::MOVE_UP_RIGHT:
                return handlePlayerMovement(1, -1, "northeast");

            case InputAction::MOVE_DOWN_LEFT:
                return handlePlayerMovement(-1, 1, "southwest");

            case InputAction::MOVE_DOWN_RIGHT:
                return handlePlayerMovement(1, 1, "southeast");

            case InputAction::WAIT:
                LOG_PLAYER("Waiting for one turn");
                game_manager->processPlayerAction(ActionSpeed::NORMAL);
                game_manager->getMessageLog()->addMessage("You wait.");
                game_manager->updateMonsters();  // Let monsters act
                return true;

            case InputAction::OPEN_DOOR:
                return handleDoorInteraction();

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