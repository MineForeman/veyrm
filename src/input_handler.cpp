#include "input_handler.h"
#include "log.h"
#include <sstream>
#include <chrono>
#include <iomanip>

InputHandler::InputHandler() {
    initializeDefaultBindings();
}

void InputHandler::initializeDefaultBindings() {
    // Arrow key movement
    keyBindings["ArrowUp"] = InputAction::MOVE_UP;
    keyBindings["ArrowDown"] = InputAction::MOVE_DOWN;
    keyBindings["ArrowLeft"] = InputAction::MOVE_LEFT;
    keyBindings["ArrowRight"] = InputAction::MOVE_RIGHT;
    
    // Numpad movement (including diagonals)
    keyBindings["1"] = InputAction::MOVE_DOWN_LEFT;
    keyBindings["2"] = InputAction::MOVE_DOWN;
    keyBindings["3"] = InputAction::MOVE_DOWN_RIGHT;
    keyBindings["4"] = InputAction::MOVE_LEFT;
    keyBindings["5"] = InputAction::WAIT;
    keyBindings["6"] = InputAction::MOVE_RIGHT;
    keyBindings["7"] = InputAction::MOVE_UP_LEFT;
    keyBindings["8"] = InputAction::MOVE_UP;
    keyBindings["9"] = InputAction::MOVE_UP_RIGHT;

    
    // Actions
    keyBindings["."] = InputAction::WAIT;
    keyBindings["o"] = InputAction::OPEN_DOOR;
    keyBindings["g"] = InputAction::GET_ITEM;
    keyBindings[">"] = InputAction::USE_STAIRS_DOWN;  // Go down stairs
    keyBindings["<"] = InputAction::USE_STAIRS_UP;    // Go up stairs
    keyBindings["Return"] = InputAction::CONFIRM;
    keyBindings["Escape"] = InputAction::CANCEL;
    keyBindings["q"] = InputAction::QUIT;
    keyBindings["Q"] = InputAction::QUIT;
    
    // UI
    keyBindings["i"] = InputAction::OPEN_INVENTORY;
    keyBindings["?"] = InputAction::OPEN_HELP;

    // Inventory actions (uppercase to avoid conflict with slot selection)
    keyBindings["u"] = InputAction::USE_ITEM;
    keyBindings["D"] = InputAction::DROP_ITEM;  // Uppercase D
    keyBindings["E"] = InputAction::EXAMINE_ITEM;  // Uppercase E

    // Save/Load
    keyBindings["S"] = InputAction::OPEN_SAVE_MENU;  // Uppercase S for save
    keyBindings["L"] = InputAction::OPEN_LOAD_MENU;  // Uppercase L for load

    // Debug
    keyBindings["F1"] = InputAction::DEBUG_TOGGLE;
}

InputAction InputHandler::processEvent(const ftxui::Event& event) {
    std::string key = eventToKey(event);

    // Log the keystroke
    std::stringstream log_msg;
    log_msg << "[INPUT] Key: '" << key << "'";

    auto it = keyBindings.find(key);
    if (it != keyBindings.end()) {
        // Log the action that this key maps to
        std::string action_name = actionToString(it->second);
        log_msg << " -> " << action_name;
        Log::input(log_msg.str());
        return it->second;
    }

    log_msg << " -> UNMAPPED";
    Log::input(log_msg.str());
    return InputAction::NONE;
}

void InputHandler::bindKey(const ftxui::Event& event, InputAction action) {
    std::string key = eventToKey(event);
    keyBindings[key] = action;
}

bool InputHandler::isMovementKey(const ftxui::Event& event) const {
    InputAction action = const_cast<InputHandler*>(this)->processEvent(event);
    return action >= InputAction::MOVE_UP && action <= InputAction::MOVE_DOWN_RIGHT;
}

bool InputHandler::isActionKey(const ftxui::Event& event) const {
    InputAction action = const_cast<InputHandler*>(this)->processEvent(event);
    return action >= InputAction::WAIT && action <= InputAction::CANCEL;
}

std::string InputHandler::eventToKey(const ftxui::Event& event) const {
    // Handle special keys
    if (event == ftxui::Event::ArrowUp) return "ArrowUp";
    if (event == ftxui::Event::ArrowDown) return "ArrowDown";
    if (event == ftxui::Event::ArrowLeft) return "ArrowLeft";
    if (event == ftxui::Event::ArrowRight) return "ArrowRight";
    if (event == ftxui::Event::Return) return "Return";
    if (event == ftxui::Event::Escape) return "Escape";
    if (event == ftxui::Event::F1) return "F1";
    
    // Handle character events
    if (event.is_character()) {
        return std::string(1, event.character()[0]);
    }
    
    // Default case
    return "";
}

std::string InputHandler::actionToString(InputAction action) const {
    switch (action) {
        case InputAction::MOVE_UP: return "MOVE_UP";
        case InputAction::MOVE_DOWN: return "MOVE_DOWN";
        case InputAction::MOVE_LEFT: return "MOVE_LEFT";
        case InputAction::MOVE_RIGHT: return "MOVE_RIGHT";
        case InputAction::MOVE_UP_LEFT: return "MOVE_UP_LEFT";
        case InputAction::MOVE_UP_RIGHT: return "MOVE_UP_RIGHT";
        case InputAction::MOVE_DOWN_LEFT: return "MOVE_DOWN_LEFT";
        case InputAction::MOVE_DOWN_RIGHT: return "MOVE_DOWN_RIGHT";
        case InputAction::WAIT: return "WAIT";
        case InputAction::QUIT: return "QUIT";
        case InputAction::CONFIRM: return "CONFIRM";
        case InputAction::CANCEL: return "CANCEL";
        case InputAction::OPEN_DOOR: return "OPEN_DOOR";
        case InputAction::GET_ITEM: return "GET_ITEM";
        case InputAction::USE_STAIRS_DOWN: return "USE_STAIRS_DOWN";
        case InputAction::USE_STAIRS_UP: return "USE_STAIRS_UP";
        case InputAction::OPEN_INVENTORY: return "OPEN_INVENTORY";
        case InputAction::OPEN_HELP: return "OPEN_HELP";
        case InputAction::USE_ITEM: return "USE_ITEM";
        case InputAction::DROP_ITEM: return "DROP_ITEM";
        case InputAction::EXAMINE_ITEM: return "EXAMINE_ITEM";
        case InputAction::OPEN_SAVE_MENU: return "OPEN_SAVE_MENU";
        case InputAction::OPEN_LOAD_MENU: return "OPEN_LOAD_MENU";
        case InputAction::DEBUG_TOGGLE: return "DEBUG_TOGGLE";
        case InputAction::NONE: return "NONE";
        default: return "UNKNOWN";
    }
}