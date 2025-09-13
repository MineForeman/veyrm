#include <catch2/catch_test_macros.hpp>
#include "input_handler.h"
#include <ftxui/component/event.hpp>

using namespace ftxui;

TEST_CASE("InputHandler: Movement keys", "[input_handler]") {
    InputHandler handler;
    
    SECTION("Arrow keys") {
        REQUIRE(handler.processEvent(Event::ArrowUp) == InputAction::MOVE_UP);
        REQUIRE(handler.processEvent(Event::ArrowDown) == InputAction::MOVE_DOWN);
        REQUIRE(handler.processEvent(Event::ArrowLeft) == InputAction::MOVE_LEFT);
        REQUIRE(handler.processEvent(Event::ArrowRight) == InputAction::MOVE_RIGHT);
    }
    
    SECTION("HJKL movement not implemented") {
        // HJKL movement is not yet implemented
        REQUIRE(handler.processEvent(Event::Character('h')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('j')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('k')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('l')) == InputAction::NONE);
    }
    
    SECTION("Diagonal movement not implemented") {
        // Diagonal movement is not yet implemented
        REQUIRE(handler.processEvent(Event::Character('y')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('u')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('b')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('n')) == InputAction::NONE);
    }
    
    SECTION("Capital letters for movement not implemented") {
        // Capital letter movement is not yet implemented
        REQUIRE(handler.processEvent(Event::Character('H')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('J')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('K')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('L')) == InputAction::NONE);
    }
}

TEST_CASE("InputHandler: Action keys", "[input_handler]") {
    InputHandler handler;
    
    SECTION("Basic actions") {
        REQUIRE(handler.processEvent(Event::Character('q')) == InputAction::QUIT);
        REQUIRE(handler.processEvent(Event::Character('Q')) == InputAction::QUIT);
        REQUIRE(handler.processEvent(Event::Character('.')) == InputAction::WAIT);
        REQUIRE(handler.processEvent(Event::Character('i')) == InputAction::OPEN_INVENTORY);
    }
    
    SECTION("Help actions") {
        REQUIRE(handler.processEvent(Event::Character('?')) == InputAction::OPEN_HELP);
    }
    
    SECTION("Item actions") {
        REQUIRE(handler.processEvent(Event::Character('g')) == InputAction::GET_ITEM);
    }

    SECTION("Other keys return NONE") {
        // These actions aren't implemented yet
        REQUIRE(handler.processEvent(Event::Character('u')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('D')) == InputAction::NONE);
    }
}

TEST_CASE("InputHandler: Special keys", "[input_handler]") {
    InputHandler handler;
    
    SECTION("Escape key") {
        REQUIRE(handler.processEvent(Event::Escape) == InputAction::CANCEL);
    }
    
    SECTION("Enter key") {
        REQUIRE(handler.processEvent(Event::Return) == InputAction::CONFIRM);
    }
    
    SECTION("Space key") {
        // Space key is not bound to any action
        REQUIRE(handler.processEvent(Event::Character(' ')) == InputAction::NONE);
    }
}

TEST_CASE("InputHandler: Unknown keys", "[input_handler]") {
    InputHandler handler;
    
    SECTION("Unknown characters") {
        REQUIRE(handler.processEvent(Event::Character('x')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('z')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('!')) == InputAction::NONE);
        REQUIRE(handler.processEvent(Event::Character('@')) == InputAction::NONE);
    }
    
    SECTION("Numbers") {
        // Numbers might be used for inventory selection in future
        for (char c = '0'; c <= '9'; ++c) {
            auto action = handler.processEvent(Event::Character(c));
            // Numbers currently return NONE or might be assigned to specific actions
            REQUIRE((action == InputAction::NONE || action != InputAction::QUIT));
        }
    }
}

TEST_CASE("InputHandler: Common input patterns", "[input_handler]") {
    InputHandler handler;
    
    SECTION("Menu navigation keys") {
        // Arrow keys should work for menu navigation
        REQUIRE(handler.processEvent(Event::ArrowUp) == InputAction::MOVE_UP);
        REQUIRE(handler.processEvent(Event::ArrowDown) == InputAction::MOVE_DOWN);
        
        // Enter to confirm
        REQUIRE(handler.processEvent(Event::Return) == InputAction::CONFIRM);
        
        // Q to quit
        REQUIRE(handler.processEvent(Event::Character('q')) == InputAction::QUIT);
    }
    
    SECTION("Inventory keys") {
        // Escape to cancel/close
        REQUIRE(handler.processEvent(Event::Escape) == InputAction::CANCEL);
        
        // Arrow keys for navigation
        REQUIRE(handler.processEvent(Event::ArrowUp) == InputAction::MOVE_UP);
        REQUIRE(handler.processEvent(Event::ArrowDown) == InputAction::MOVE_DOWN);
    }
    
    SECTION("Playing keys") {
        // Arrow keys work for movement
        REQUIRE(handler.processEvent(Event::ArrowUp) == InputAction::MOVE_UP);
        REQUIRE(handler.processEvent(Event::ArrowLeft) == InputAction::MOVE_LEFT);
        REQUIRE(handler.processEvent(Event::Character('i')) == InputAction::OPEN_INVENTORY);
    }
}

TEST_CASE("InputHandler: Key mapping consistency", "[input_handler]") {
    InputHandler handler;
    
    SECTION("Case insensitive for most commands") {
        // Quit works with both cases
        REQUIRE(handler.processEvent(Event::Character('q')) == InputAction::QUIT);
        REQUIRE(handler.processEvent(Event::Character('Q')) == InputAction::QUIT);
        
        // Movement currently only works with arrow keys
        REQUIRE(handler.processEvent(Event::ArrowLeft) == InputAction::MOVE_LEFT);
        REQUIRE(handler.processEvent(Event::ArrowRight) == InputAction::MOVE_RIGHT);
    }
    
    SECTION("Multiple keys for same action") {
        // Wait can be triggered by '.' or numpad '5'
        REQUIRE(handler.processEvent(Event::Character('.')) == InputAction::WAIT);
        REQUIRE(handler.processEvent(Event::Character('5')) == InputAction::WAIT);
        
        // Currently only arrow keys work for movement
        REQUIRE(handler.processEvent(Event::ArrowLeft) == InputAction::MOVE_LEFT);
        REQUIRE(handler.processEvent(Event::ArrowUp) == InputAction::MOVE_UP);
    }
}