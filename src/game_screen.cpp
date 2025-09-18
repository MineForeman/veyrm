#include "game_screen.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "map_generator.h"
#include "renderer.h"
#include "status_bar.h"
#include "layout_system.h"
#include "inventory_renderer.h"
#include "log.h"
#include "ecs/entity.h"
#include "ecs/position_component.h"
#include "ecs/inventory_component.h"
#include "ecs/item_component.h"
#include "ecs/inventory_system.h"
#include "ecs/game_world.h"
#include "ecs/combat_system.h"
#include "ecs/position_component.h"
#include "ecs/event.h"
#include "controllers/game_controller.h"
#include "ui/game_view.h"
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
      layout_system(std::make_unique<LayoutSystem>()),
      inventory_renderer(nullptr),
      controller(nullptr),
      view(nullptr) {
    // Center renderer on player's starting position
    renderer->centerOn(game_manager->player_x, game_manager->player_y);

    // Initialize MVC components
    if (game_manager) {
        auto* ecs_world = game_manager->getECSWorld();
        controller = std::make_unique<controllers::GameController>(game_manager, ecs_world);
        view = std::make_unique<ui::GameView>(game_manager, screen);

        // Set up controller callbacks
        controllers::GameController::ViewCallbacks callbacks;
        callbacks.refreshDisplay = []() {
            // Display updates are handled automatically by FTXUI
        };
        callbacks.showMessage = [this](const std::string& msg) {
            if (auto* msg_log = game_manager->getMessageLog()) {
                msg_log->addMessage(msg);
            }
        };
        callbacks.showPrompt = [this](const std::string& prompt) {
            if (auto* msg_log = game_manager->getMessageLog()) {
                msg_log->addMessage(prompt);
            }
        };
        callbacks.clearPrompt = []() {
            // Prompts clear automatically with new messages
        };
        callbacks.exitToMenu = [this]() {
            game_manager->setState(GameState::MENU);
        };
        controller->setViewCallbacks(callbacks);
    }
}

GameScreen::~GameScreen() = default;

void GameScreen::setAuthenticationInfo(int user_id, const std::string& session_token) {
    auth_user_id = user_id;
    auth_session_token = session_token;

    // Also update the GameManager
    if (game_manager) {
        game_manager->setAuthenticationInfo(user_id, session_token);
    }
}

bool GameScreen::handleDoorInteraction() {
    auto* msg_log = game_manager->getMessageLog();
    if (!msg_log) {
        return false;
    }

    // Set state to await direction input
    awaiting_direction = true;
    direction_prompt = "Open door in which direction?";
    msg_log->addMessage(direction_prompt);

    LOG_PLAYER("Prompting for door direction");
    return true; // Input handled, now waiting for direction
}

bool GameScreen::handleDirectionalDoorInteraction(int dx, int dy) {
    auto* map = game_manager->getMap();
    auto* msg_log = game_manager->getMessageLog();

    if (!map || !msg_log) {
        return false;
    }

    // Calculate target position using game_manager position
    int target_x = game_manager->player_x + dx;
    int target_y = game_manager->player_y + dy;

    if (!map->inBounds(target_x, target_y)) {
        msg_log->addMessage("There is no door there.");
        return false;
    }

    TileType tile = map->getTile(target_x, target_y);

    if (tile == TileType::DOOR_CLOSED) {
        // Open the door
        map->setTile(target_x, target_y, TileType::DOOR_OPEN);
        msg_log->addMessage("You open the door.");
        LOG_ENVIRONMENT("Door opened at (" + std::to_string(target_x) + ", " + std::to_string(target_y) + ")");
        LOG_PLAYER("Player opened a closed door");

        // Opening doors takes a turn
        game_manager->processPlayerAction(ActionSpeed::NORMAL);
        game_manager->updateMonsters();

        return true;
    } else if (tile == TileType::DOOR_OPEN) {
        // Close the door
        map->setTile(target_x, target_y, TileType::DOOR_CLOSED);
        msg_log->addMessage("You close the door.");
        LOG_ENVIRONMENT("Door closed at (" + std::to_string(target_x) + ", " + std::to_string(target_y) + ")");
        LOG_PLAYER("Player closed an open door");

        // Closing doors takes a turn
        game_manager->processPlayerAction(ActionSpeed::NORMAL);
        game_manager->updateMonsters();

        return true;
    } else {
        msg_log->addMessage("There is no door there.");
        LOG_PLAYER("Player tried to interact with door but none found at specified location");
        return false;
    }
}

bool GameScreen::handleStairInteraction(bool going_down) {
    auto* map = game_manager->getMap();
    auto* msg_log = game_manager->getMessageLog();

    if (!map || !msg_log) {
        return false;
    }

    // Check if player is standing on the appropriate stairs
    int player_x = game_manager->player_x;
    int player_y = game_manager->player_y;
    TileType current_tile = map->getTile(player_x, player_y);

    TileType required_stairs = going_down ? TileType::STAIRS_DOWN : TileType::STAIRS_UP;

    if (current_tile != required_stairs) {
        std::string stair_type = going_down ? "down" : "up";
        msg_log->addMessage("You must be standing on stairs " + stair_type + " to use them.");
        return false;
    }

    // Handle level transition
    int current_depth = game_manager->getCurrentDepth();
    int new_depth = going_down ? current_depth + 1 : current_depth - 1;

    // Prevent going up from level 1
    if (new_depth < 1) {
        msg_log->addMessage("You cannot go higher than the first level.");
        return false;
    }

    // Perform level transition
    std::string direction_text = going_down ? "descend" : "ascend";
    std::string level_text = going_down ? "deeper into" : "back up through";

    msg_log->addMessage("You " + direction_text + " the stairs, going " + level_text + " the dungeon...");
    LOG_PLAYER(std::string("Player used stairs to go ") + (going_down ? "down" : "up") + " from depth " +
               std::to_string(current_depth) + " to " + std::to_string(new_depth));

    // Change depth and regenerate map
    game_manager->setCurrentDepth(new_depth);

    // Generate new map with depth-specific seed
    unsigned int depth_seed = game_manager->getSeedForDepth(new_depth);
    game_manager->setCurrentMapSeed(depth_seed);
    game_manager->initializeMap(game_manager->getCurrentMapType());

    // Clear exploration/visibility from previous level
    map->clearExploration();
    map->clearVisibility();

    // Place player on opposite stairs type
    // When going down, player should appear at stairs UP
    // When going up, player should appear at stairs DOWN
    TileType target_stairs = going_down ? TileType::STAIRS_UP : TileType::STAIRS_DOWN;
    Point stairs_pos(-1, -1);

    // Find the opposite stairs position
    for (int y = 0; y < map->getHeight() && stairs_pos.x == -1; y++) {
        for (int x = 0; x < map->getWidth(); x++) {
            if (map->getTile(x, y) == target_stairs) {
                stairs_pos = Point(x, y);
                break;
            }
        }
    }

    // Set player position
    if (stairs_pos.x != -1 && stairs_pos.y != -1) {
        game_manager->player_x = stairs_pos.x;
        game_manager->player_y = stairs_pos.y;
    } else {
        // Fallback to spawn point if stairs not found
        auto spawn_point = MapGenerator::getDefaultSpawnPoint(game_manager->getCurrentMapType());
        game_manager->player_x = spawn_point.x;
        game_manager->player_y = spawn_point.y;
    }

    // Update ECS player position if in ECS mode
    if (game_manager->isECSMode()) {
        auto* ecs_world = game_manager->getECSWorld();
        if (ecs_world) {
            auto* player_entity = ecs_world->getPlayerEntity();
            if (player_entity) {
                auto* pos_comp = player_entity->getComponent<ecs::PositionComponent>();
                if (pos_comp) {
                    pos_comp->moveTo(game_manager->player_x, game_manager->player_y);
                }
            }
        }
    }

    // Update FOV for new level (entities already spawned in initializeMap)
    game_manager->updateFOV();

    // Using stairs takes a turn
    game_manager->processPlayerAction(ActionSpeed::NORMAL);
    game_manager->updateMonsters();

    std::string depth_msg = "Welcome to dungeon level " + std::to_string(new_depth) + "!";
    msg_log->addMessage(depth_msg);

    return true;
}

bool GameScreen::handlePlayerMovement(int dx, int dy, const std::string& direction) {
    LOG_DEBUG("handlePlayerMovement called: dx=" + std::to_string(dx) + ", dy=" + std::to_string(dy) + ", dir=" + direction);

    auto ecs_world = game_manager->getECSWorld();
    if (!ecs_world) {
        LOG_ERROR("ECS world not available");
        return false;
    }

    LOG_DEBUG("Using ECS movement system for " + direction);
    ActionSpeed speed = ecs_world->processPlayerAction(0, dx, dy);

    // Sync the ECS player position
    auto player_entity = ecs_world->getPlayerEntity();
    if (player_entity) {
        auto* pos = player_entity->getComponent<ecs::PositionComponent>();
        if (pos) {
            // Update the game manager's position variables
            game_manager->player_x = pos->position.x;
            game_manager->player_y = pos->position.y;
            LOG_DEBUG("ECS player moved to: (" + std::to_string(pos->position.x) +
                     ", " + std::to_string(pos->position.y) + ")");
        }
    }

    game_manager->processPlayerAction(speed);

    // Update FOV and renderer
    if (player_entity && renderer) {
        auto* pos = player_entity->getComponent<ecs::PositionComponent>();
        if (pos) {
            renderer->centerOn(pos->position.x, pos->position.y);
        }
    }
    game_manager->updateFOV();
    game_manager->updateMonsters();

    // Auto-save after successful movement to preserve current position
    game_manager->autoSave();

    return true;
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

Component GameScreen::CreateInventoryPanel() {
    return Renderer([this] {
        // Initialize inventory renderer if needed
        if (!inventory_renderer) {
            if (auto* player = game_manager->getPlayer()) {
                inventory_renderer = std::make_unique<InventoryRenderer>(player);
                // Set ECS world if available
                if (auto* ecs_world = game_manager->getECSWorld()) {
                    inventory_renderer->setECSWorld(ecs_world);
                }
            }
        }

        if (inventory_renderer) {
            return inventory_renderer->render() | center;
        }

        return text("Inventory not available") | center;
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
    auto inventory_panel = CreateInventoryPanel();

    // Create structured layout with proper separation
    // The key is to use Container::Horizontal to properly separate left and right
    auto right_panel = Container::Vertical({
        status_panel,
        log_panel
    });

    auto game_layout = Container::Horizontal({
        map_panel,     // Map on the left
        right_panel    // Status and log on the right
    });

    // Combine both layouts - let the renderer decide which to show
    auto combined_layout = Container::Vertical({
        game_layout,
        inventory_panel
    });

    // Create a renderer that switches between game and inventory display
    auto layout = Renderer(combined_layout, [this, game_layout, inventory_panel] {
        if (game_manager->getState() == GameState::INVENTORY) {
            return inventory_panel->Render();
        } else {
            return game_layout->Render();
        }
    });

    // Add input handling
    layout = CatchEvent(layout, [this](Event event) {
        // Use the controller if available for MVC pattern
        if (controller) {
            bool controller_handled = controller->handleInput(event);
            LOG_DEBUG("Controller returned: " + std::string(controller_handled ? "true" : "false"));
            if (controller_handled) {
                return true;
            }
        }

        // Fallback to direct input handling if controller not available
        LOG_DEBUG("Fallback input handling triggered");
        InputHandler* input = game_manager->getInputHandler();
        InputAction action = input->processEvent(event);

        // Log the input for debugging
        if (event.is_character()) {
            LOG_DEBUG("Character input: " + event.character());
        }
        LOG_DEBUG("Action: " + std::to_string(static_cast<int>(action)) +
                  ", State: " + std::to_string(static_cast<int>(game_manager->getState())));


        // Handle inventory-specific input when in inventory state
        if (game_manager->getState() == GameState::INVENTORY) {
            return handleInventoryInput(action, event);
        }

        switch(action) {
            case InputAction::QUIT:
                game_manager->setState(GameState::MENU);
                return true;

            case InputAction::CANCEL:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    auto* msg_log = game_manager->getMessageLog();
                    if (msg_log) {
                        msg_log->addMessage("Cancelled.");
                    }
                    return true;
                }
                // If not awaiting direction, do nothing
                return false;

            case InputAction::MOVE_UP:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(0, -1);
                }
                return handlePlayerMovement(0, -1, "north");

            case InputAction::MOVE_DOWN:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(0, 1);
                }
                return handlePlayerMovement(0, 1, "south");

            case InputAction::MOVE_LEFT:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(-1, 0);
                }
                return handlePlayerMovement(-1, 0, "west");

            case InputAction::MOVE_RIGHT:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(1, 0);
                }
                return handlePlayerMovement(1, 0, "east");

            case InputAction::MOVE_UP_LEFT:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(-1, -1);
                }
                return handlePlayerMovement(-1, -1, "northwest");

            case InputAction::MOVE_UP_RIGHT:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(1, -1);
                }
                return handlePlayerMovement(1, -1, "northeast");

            case InputAction::MOVE_DOWN_LEFT:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(-1, 1);
                }
                return handlePlayerMovement(-1, 1, "southwest");

            case InputAction::MOVE_DOWN_RIGHT:
                if (awaiting_direction) {
                    awaiting_direction = false;
                    return handleDirectionalDoorInteraction(1, 1);
                }
                return handlePlayerMovement(1, 1, "southeast");

            case InputAction::WAIT:
                LOG_PLAYER("Waiting for one turn");
                game_manager->processPlayerAction(ActionSpeed::NORMAL);
                game_manager->getMessageLog()->addMessage("You wait.");
                game_manager->updateMonsters();  // Let monsters act
                return true;

            case InputAction::OPEN_DOOR:
                return handleDoorInteraction();

            case InputAction::GET_ITEM: {
                auto* ecs_world = game_manager->getECSWorld();
                if (ecs_world) {
                    // Use ECS action system for item pickup (action 1 = pickup)
                    ActionSpeed speed = ecs_world->processPlayerAction(1, 0, 0);
                    game_manager->processPlayerAction(speed);
                }
                return true;
            }

            case InputAction::USE_STAIRS_DOWN:
                return handleStairInteraction(true);  // Going down

            case InputAction::USE_STAIRS_UP:
                return handleStairInteraction(false); // Going up

            case InputAction::OPEN_INVENTORY:
                game_manager->setState(GameState::INVENTORY);
                return true;

            case InputAction::OPEN_HELP:
                game_manager->setState(GameState::HELP);
                return true;

            case InputAction::OPEN_SAVE_MENU:
                LOG_INFO("Save menu triggered");
                game_manager->setSaveMenuMode(true);
                // Auto-save and return to menu instead of save/load screen
game_manager->autoSave();
game_manager->setState(GameState::MENU);
                return true;

            case InputAction::OPEN_LOAD_MENU:
                LOG_INFO("Load menu triggered");
                game_manager->setSaveMenuMode(false);
                // Auto-save and return to menu instead of save/load screen
game_manager->autoSave();
game_manager->setState(GameState::MENU);
                return true;

            default:
                break;
        }
        
        return false;
    });

    return layout;
}

bool GameScreen::handleInventoryInput(InputAction action, const ftxui::Event& event) {
    LOG_INFO("handleInventoryInput: action=" + std::to_string(static_cast<int>(action)));

    if (!inventory_renderer) {
        // Initialize if needed
        if (auto* player = game_manager->getPlayer()) {
            inventory_renderer = std::make_unique<InventoryRenderer>(player);
            // Set ECS world for inventory access
            if (auto* ecs_world = game_manager->getECSWorld()) {
                inventory_renderer->setECSWorld(ecs_world);
            }
            LOG_INFO("Created inventory renderer");
        }
        if (!inventory_renderer) {
            LOG_ERROR("Failed to create inventory renderer");
            return false;
        }
    }

    // Check for inventory-specific action keys first
    if (event.is_character()) {
        char c = event.character()[0];

        // Handle inventory action keys (case-insensitive)
        if (c == 'd' || c == 'D') {
            LOG_INFO("Inventory key: '" + std::string(1, c) + "' -> DROP_ITEM");
            action = InputAction::DROP_ITEM;
        } else if (c == 'e' || c == 'E' || c == 'x' || c == 'X') {
            LOG_INFO("Inventory key: '" + std::string(1, c) + "' -> EXAMINE_ITEM");
            action = InputAction::EXAMINE_ITEM;
        } else if (c == 'u' || c == 'U') {
            LOG_INFO("Inventory key: '" + std::string(1, c) + "' -> USE_ITEM");
            action = InputAction::USE_ITEM;
        } else if (c >= 'a' && c <= 'z') {
            // Only treat as slot selection if not an action key
            int slot = c - 'a';
            LOG_INFO("Inventory slot selection: '" + std::string(1, c) + "' -> slot " + std::to_string(slot));
            inventory_renderer->selectSlot(slot);
            return true;
        }
    }

    switch (action) {
        case InputAction::OPEN_INVENTORY:  // Toggle close with 'i'
            LOG_INFO("Closing inventory with 'i'");
            game_manager->setState(GameState::PLAYING);
            inventory_renderer->reset();
            return true;

        case InputAction::CANCEL:           // Close with ESC
            LOG_INFO("Closing inventory with ESC");
            game_manager->setState(GameState::PLAYING);
            inventory_renderer->reset();
            return true;

        case InputAction::MOVE_UP:
            LOG_INFO("Moving selection up");
            inventory_renderer->selectPrevious();
            return true;

        case InputAction::MOVE_DOWN:
            LOG_INFO("Moving selection down");
            inventory_renderer->selectNext();
            return true;

        case InputAction::USE_ITEM: {
            auto* msg_log = game_manager->getMessageLog();
            auto* ecs_world = game_manager->getECSWorld();

            if (ecs_world && inventory_renderer) {
                auto* selected_item = static_cast<ecs::Entity*>(inventory_renderer->getSelectedItem());
                if (selected_item) {
                    auto* inv_system = ecs_world->getInventorySystem();
                    if (inv_system) {
                        auto player_id = ecs_world->getPlayerID();
                        auto* player_entity = ecs_world->getEntity(player_id);
                        if (inv_system->useItem(player_entity, selected_item->getID())) {
                            msg_log->addMessage("You use the item.");
                            // Refresh inventory display
                            inventory_renderer->reset();
                        } else {
                            msg_log->addMessage("Cannot use that item.");
                        }
                    }
                } else {
                    msg_log->addMessage("No item selected.");
                }
            }
            return true;
        }

        case InputAction::DROP_ITEM: {
            auto* msg_log = game_manager->getMessageLog();
            auto* ecs_world = game_manager->getECSWorld();

            if (ecs_world && inventory_renderer) {
                auto* selected_item = static_cast<ecs::Entity*>(inventory_renderer->getSelectedItem());
                if (selected_item) {
                    auto* inv_system = ecs_world->getInventorySystem();
                    if (inv_system) {
                        auto player_id = ecs_world->getPlayerID();
                        auto* player_entity = ecs_world->getEntity(player_id);
                        if (inv_system->dropItem(player_entity, selected_item->getID())) {
                            msg_log->addMessage("You drop the item.");
                            // Process drop event immediately
                            ecs::EventSystem::getInstance().update();
                            // Refresh inventory display
                            inventory_renderer->reset();
                        } else {
                            msg_log->addMessage("Cannot drop that item.");
                        }
                    }
                } else {
                    msg_log->addMessage("No item selected.");
                }
            }
            return true;
        }

        case InputAction::EXAMINE_ITEM: {
            auto* msg_log = game_manager->getMessageLog();
            if (msg_log) {
                auto* ecs_world = game_manager->getECSWorld();
                if (ecs_world && inventory_renderer) {
                    auto* selected_item = static_cast<ecs::Entity*>(inventory_renderer->getSelectedItem());
                    if (selected_item) {
                        auto* item_comp = selected_item->getComponent<ecs::ItemComponent>();
                        if (item_comp) {
                            msg_log->addMessage("Examining: " + item_comp->name);
                            if (!item_comp->description.empty()) {
                                msg_log->addMessage(item_comp->description);
                            }
                            if (item_comp->heal_amount > 0) {
                                msg_log->addMessage("Heals " + std::to_string(item_comp->heal_amount) + " HP when used.");
                            }
                        }
                    } else {
                        msg_log->addMessage("No item selected.");
                    }
                }
            }
            return true;
        }

        default:
            break;
    }

    return false;
}