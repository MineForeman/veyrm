#include "game_screen.h"
#include "item_factory.h"
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
#include "inventory_renderer.h"
#include "log.h"
#include "monster.h"
#include "combat_system.h"
#include "item_manager.h"
#include "item.h"
#include "ecs/game_world.h"
#include "ecs/position_component.h"
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
      inventory_renderer(nullptr) {
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
    LOG_DEBUG("handlePlayerMovement called: dx=" + std::to_string(dx) + ", dy=" + std::to_string(dy) + ", dir=" + direction);
    std::cout << "[handlePlayerMovement] Called with dx=" << dx << ", dy=" << dy << ", dir=" << direction << std::endl;

    // Use ECS movement if ECS mode is enabled
    bool ecs_mode = game_manager->isECSMode();
    std::cout << "[handlePlayerMovement] ECS mode: " << ecs_mode << std::endl;

    if (ecs_mode) {
        auto ecs_world = game_manager->getECSWorld();
        std::cout << "[handlePlayerMovement] ECS world exists: " << (ecs_world != nullptr) << std::endl;
        if (ecs_world) {
            LOG_DEBUG("Using ECS movement system for " + direction);
            std::cout << "[handlePlayerMovement] Calling ECS processPlayerAction" << std::endl;
            ActionSpeed speed = ecs_world->processPlayerAction(0, dx, dy);

            // Immediately sync the ECS player position to deprecated variables
            auto player_entity = ecs_world->getPlayerEntity();
            if (player_entity) {
                auto* pos = player_entity->getComponent<ecs::PositionComponent>();
                if (pos) {
                    // Update the game manager's deprecated position variables
                    game_manager->player_x = pos->position.x;
                    game_manager->player_y = pos->position.y;
                    LOG_DEBUG("ECS player moved to: (" + std::to_string(pos->position.x) +
                             ", " + std::to_string(pos->position.y) + ")");
                }
            }

            game_manager->processPlayerAction(speed);

            // Update FOV and renderer
            // Get updated player position from ECS (which should now be synced)
            auto* player = game_manager->getPlayer();
            if (player && renderer) {
                renderer->centerOn(player->x, player->y);
            }
            game_manager->updateFOV();
            game_manager->updateMonsters();
            return true;
        }
    }

    // Legacy movement code
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

Component GameScreen::CreateInventoryPanel() {
    return Renderer([this] {
        // Initialize inventory renderer if needed
        if (!inventory_renderer) {
            if (auto* player = game_manager->getPlayer()) {
                inventory_renderer = std::make_unique<InventoryRenderer>(player);
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
        InputHandler* input = game_manager->getInputHandler();
        InputAction action = input->processEvent(event);

        // Log the input for debugging
        if (event.is_character()) {
            LOG_DEBUG("Character input: " + event.character());
            std::cout << "[GameScreen] Character input: '" << event.character() << "'" << std::endl;
        }
        LOG_DEBUG("Action: " + std::to_string(static_cast<int>(action)) +
                  ", State: " + std::to_string(static_cast<int>(game_manager->getState())));
        std::cout << "[GameScreen] Action: " << static_cast<int>(action)
                  << ", State: " << static_cast<int>(game_manager->getState()) << std::endl;


        // Handle inventory-specific input when in inventory state
        if (game_manager->getState() == GameState::INVENTORY) {
            return handleInventoryInput(action, event);
        }

        switch(action) {
            case InputAction::QUIT:
                game_manager->setState(GameState::MENU);
                return true;

            case InputAction::MOVE_UP:
                std::cout << "[GameScreen] MOVE_UP action received!" << std::endl;
                return handlePlayerMovement(0, -1, "north");

            case InputAction::MOVE_DOWN:
                std::cout << "[GameScreen] MOVE_DOWN action received!" << std::endl;
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

            case InputAction::GET_ITEM: {
                auto* player = game_manager->getPlayer();
                auto* item_manager = game_manager->getItemManager();
                auto* msg_log = game_manager->getMessageLog();

                if (player && item_manager) {
                    auto item = item_manager->getItemAt(player->x, player->y);
                    if (item) {
                        // Create a copy for inventory
                        auto item_copy = ItemFactory::getInstance().create(item->id);
                        if (item_copy) {
                            // Copy stack size and properties
                            item_copy->stack_size = item->stack_size;
                            item_copy->properties = item->properties;

                            // Try to pick up the item
                            if (player->pickupItem(std::move(item_copy))) {
                                msg_log->addMessage("You pick up " + item->name + ".");

                                // Special message for gold
                                if (item->type == Item::ItemType::GOLD) {
                                    msg_log->addMessage("You gain " + std::to_string(item->properties["amount"]) + " gold.");
                                }

                                // Remove from world
                                item_manager->removeItem(item);
                                game_manager->processPlayerAction(ActionSpeed::FAST);
                                return true;
                            } else {
                                msg_log->addMessage("Your inventory is full!");
                            }
                        }
                    } else {
                        msg_log->addMessage("There is nothing here to pick up.");
                    }
                }
                return false;
            }

            case InputAction::OPEN_INVENTORY:
                game_manager->setState(GameState::INVENTORY);
                return true;

            case InputAction::OPEN_HELP:
                game_manager->setState(GameState::HELP);
                return true;

            case InputAction::OPEN_SAVE_MENU:
                LOG_INFO("Save menu triggered");
                game_manager->setSaveMenuMode(true);
                game_manager->setState(GameState::SAVE_LOAD);
                return true;

            case InputAction::OPEN_LOAD_MENU:
                LOG_INFO("Load menu triggered");
                game_manager->setSaveMenuMode(false);
                game_manager->setState(GameState::SAVE_LOAD);
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
            LOG_INFO("Created inventory renderer");
        }
        if (!inventory_renderer) {
            LOG_ERROR("Failed to create inventory renderer");
            return false;
        }
    }

    // Handle direct slot selection with letter keys
    // Now includes 'd' and 'e' since we use uppercase D and E for actions
    if (event.is_character()) {
        char c = event.character()[0];
        if (c >= 'a' && c <= 'z') {
            int slot = c - 'a';
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
            auto* player = game_manager->getPlayer();
            auto* msg_log = game_manager->getMessageLog();
            Item* item = inventory_renderer->getSelectedItem();

            if (item && player && msg_log) {
                // Handle potion use
                if (item->type == Item::ItemType::POTION) {
                    if (item->properties.count("heal")) {
                        int heal_amount = item->properties.at("heal");
                        player->heal(heal_amount);
                        msg_log->addMessage("You drink the " + item->name + " and recover " +
                                          std::to_string(heal_amount) + " HP.");

                        // Remove the item
                        player->inventory->removeItem(inventory_renderer->getSelectedSlot());

                        // Close inventory and consume turn
                        game_manager->setState(GameState::PLAYING);
                        game_manager->processPlayerAction(ActionSpeed::NORMAL);
                        inventory_renderer->reset();
                        return true;
                    }
                }
                msg_log->addMessage("You can't use that item.");
            }
            return true;
        }

        case InputAction::DROP_ITEM: {
            auto* player = game_manager->getPlayer();
            auto* item_manager = game_manager->getItemManager();
            auto* msg_log = game_manager->getMessageLog();

            if (player && item_manager && msg_log) {
                auto dropped = player->inventory->removeItem(inventory_renderer->getSelectedSlot());
                if (dropped) {
                    std::string item_name = dropped->name;
                    int px = player->x;
                    int py = player->y;

                    msg_log->addMessage("You drop the " + item_name + ".");
                    LOG_INFO("Dropping item: " + item_name + " at (" +
                             std::to_string(px) + ", " + std::to_string(py) + ")");

                    // Spawn item at player position
                    item_manager->spawnItem(std::move(dropped), px, py);

                    // Log item count after drop
                    LOG_INFO("Items in world after drop: " +
                             std::to_string(item_manager->getItemCount()));

                    // Consume turn
                    game_manager->processPlayerAction(ActionSpeed::FAST);
                }
            }
            return true;
        }

        case InputAction::EXAMINE_ITEM: {
            auto* msg_log = game_manager->getMessageLog();
            Item* item = inventory_renderer->getSelectedItem();

            if (item && msg_log) {
                msg_log->addMessage("Examining: " + item->name);
                if (!item->description.empty()) {
                    msg_log->addMessage(item->description);
                }
                // Add more detailed info
                if (item->type == Item::ItemType::POTION && item->properties.count("heal")) {
                    msg_log->addMessage("Heals " + std::to_string(item->properties.at("heal")) + " HP when used.");
                }
            }
            return true;
        }

        default:
            break;
    }

    return false;
}