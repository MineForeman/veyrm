#pragma once

#include <ftxui/component/event.hpp>
#include <functional>
#include <unordered_map>

// Forward declarations
class GameManager;

// Input action types
enum class InputAction {
    // Movement
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP_LEFT,
    MOVE_UP_RIGHT,
    MOVE_DOWN_LEFT,
    MOVE_DOWN_RIGHT,
    
    // Game actions
    WAIT,
    QUIT,
    CONFIRM,
    CANCEL,
    OPEN_DOOR,  // Open/close doors
    GET_ITEM,   // Pick up items

    // UI actions
    OPEN_INVENTORY,
    OPEN_HELP,

    // Inventory actions
    USE_ITEM,
    DROP_ITEM,
    EXAMINE_ITEM,

    // Save/Load actions
    OPEN_SAVE_MENU,
    OPEN_LOAD_MENU,

    // Debug
    DEBUG_TOGGLE,

    NONE
};

class InputHandler {
public:
    InputHandler();
    
    // Process input event and return the corresponding action
    InputAction processEvent(const ftxui::Event& event);
    
    // Register custom key bindings
    void bindKey(const ftxui::Event& event, InputAction action);
    
    // Check if an event matches a specific key
    bool isMovementKey(const ftxui::Event& event) const;
    bool isActionKey(const ftxui::Event& event) const;
    
private:
    // Default key bindings
    void initializeDefaultBindings();
    
    // Key mapping
    std::unordered_map<std::string, InputAction> keyBindings;
    
    // Convert event to string key for mapping
    std::string eventToKey(const ftxui::Event& event) const;
};