#include "input_handler.h"
#include <sstream>

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
    keyBindings["Return"] = InputAction::CONFIRM;
    keyBindings["Escape"] = InputAction::CANCEL;
    keyBindings["q"] = InputAction::QUIT;
    keyBindings["Q"] = InputAction::QUIT;
    
    // UI
    keyBindings["i"] = InputAction::OPEN_INVENTORY;
    keyBindings["?"] = InputAction::OPEN_HELP;
    
    // Debug
    keyBindings["F1"] = InputAction::DEBUG_TOGGLE;
}

InputAction InputHandler::processEvent(const ftxui::Event& event) {
    std::string key = eventToKey(event);
    
    auto it = keyBindings.find(key);
    if (it != keyBindings.end()) {
        return it->second;
    }
    
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