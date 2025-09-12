#include <catch2/catch_test_macros.hpp>
#include "message_log.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

TEST_CASE("MessageLog: Basic message addition", "[message_log]") {
    MessageLog log;
    
    SECTION("Add single message") {
        log.addMessage("Test message");
        
        auto messages = log.getRecentMessages(1);
        REQUIRE(messages.size() == 1);
        REQUIRE(messages[0] == "Test message");
    }
    
    SECTION("Add multiple messages") {
        log.addMessage("First message");
        log.addMessage("Second message");
        log.addMessage("Third message");
        
        auto messages = log.getRecentMessages(3);
        REQUIRE(messages.size() == 3);
        // getRecentMessages returns oldest first
        REQUIRE(messages[0] == "First message");
        REQUIRE(messages[1] == "Second message");
        REQUIRE(messages[2] == "Third message");
    }
}

TEST_CASE("MessageLog: Message types", "[message_log]") {
    MessageLog log;
    
    SECTION("Combat messages") {
        log.addCombatMessage("You hit the goblin!");
        
        auto messages = log.getRecentMessages(1);
        REQUIRE(messages.size() == 1);
        REQUIRE(messages[0] == "[Combat] You hit the goblin!");
    }
    
    SECTION("System messages") {
        log.addSystemMessage("Game saved.");
        
        auto messages = log.getRecentMessages(1);
        REQUIRE(messages.size() == 1);
        REQUIRE(messages[0] == "[System] Game saved.");
    }
    
    SECTION("Mixed message types") {
        log.addMessage("Normal message");
        log.addSystemMessage("System message");
        log.addCombatMessage("Combat message");
        
        auto messages = log.getRecentMessages(3);
        REQUIRE(messages.size() == 3);
        // Oldest first
        REQUIRE(messages[0] == "Normal message");
        REQUIRE(messages[1] == "[System] System message");
        REQUIRE(messages[2] == "[Combat] Combat message");
    }
}

TEST_CASE("MessageLog: Message limit", "[message_log]") {
    MessageLog log;
    
    SECTION("Request fewer messages than exist") {
        for (int i = 0; i < 10; ++i) {
            log.addMessage("Message " + std::to_string(i));
        }
        
        auto messages = log.getRecentMessages(5);
        REQUIRE(messages.size() == 5);
        
        // Should get the 5 most recent messages (oldest first)
        REQUIRE(messages[0] == "Message 5");  // 5th most recent
        REQUIRE(messages[4] == "Message 9");  // Most recent
    }
    
    SECTION("Request more messages than exist") {
        log.addMessage("Only message");
        
        auto messages = log.getRecentMessages(10);
        // MessageLog constructor adds 2 messages, plus we added 1 = 3 total
        REQUIRE(messages.size() == 3);
        REQUIRE(messages[2] == "Only message");  // Most recent message
    }
    
    SECTION("Maximum message history") {
        // Add many messages (default max is 100)
        for (int i = 0; i < 200; ++i) {
            log.addMessage("Message " + std::to_string(i));
        }
        
        // Should still be able to get recent messages
        auto messages = log.getRecentMessages(5);
        REQUIRE(messages.size() == 5);
        // getRecentMessages returns oldest first, so messages[0] is the 5th most recent
        REQUIRE(messages[0] == "Message 195");  // 5th most recent of the last 5
        REQUIRE(messages[4] == "Message 199");  // Most recent
    }
}

TEST_CASE("MessageLog: Clear messages", "[message_log]") {
    MessageLog log;
    
    SECTION("Clear all messages") {
        log.addMessage("Message 1");
        log.addMessage("Message 2");
        log.addMessage("Message 3");
        
        log.clear();
        
        auto messages = log.getRecentMessages(10);
        // clear() adds a "Message log cleared." message
        REQUIRE(messages.size() == 1);
        REQUIRE(messages[0] == "Message log cleared.");
    }
    
    SECTION("Add after clear") {
        log.addMessage("Old message");
        log.clear();
        log.addMessage("New message");
        
        auto messages = log.getRecentMessages(10);
        // clear() adds a message, then we add another
        REQUIRE(messages.size() == 2);
        REQUIRE(messages[0] == "Message log cleared.");
        REQUIRE(messages[1] == "New message");
    }
}

TEST_CASE("MessageLog: Render output", "[message_log]") {
    MessageLog log;
    
    SECTION("Render empty log") {
        auto element = log.render(5);
        REQUIRE(element != nullptr);  // Should return valid element even when empty
    }
    
    SECTION("Render with messages") {
        log.addMessage("Line 1");
        log.addMessage("Line 2");
        log.addMessage("Line 3");
        
        auto element = log.render(5);
        REQUIRE(element != nullptr);
        
        // Render returns an FTXUI element, not testable for content here
        // but we can verify it doesn't crash
    }
    
    SECTION("Render limited lines") {
        for (int i = 0; i < 10; ++i) {
            log.addMessage("Message " + std::to_string(i));
        }
        
        auto element = log.render(3);  // Only show 3 lines
        REQUIRE(element != nullptr);
    }
}

TEST_CASE("MessageLog: Special characters", "[message_log]") {
    MessageLog log;
    
    SECTION("Unicode characters") {
        log.addMessage("Unicode: ♠♣♥♦");
        log.addMessage("Arrows: ←↑→↓");
        log.addMessage("Box: ┌─┐│└┘");
        
        auto messages = log.getRecentMessages(3);
        // Oldest first
        REQUIRE(messages[0] == "Unicode: ♠♣♥♦");
        REQUIRE(messages[1] == "Arrows: ←↑→↓");
        REQUIRE(messages[2] == "Box: ┌─┐│└┘");
    }
    
    SECTION("Empty messages") {
        log.addMessage("");
        
        auto messages = log.getRecentMessages(1);
        REQUIRE(messages[0] == "");
    }
    
    SECTION("Very long messages") {
        std::string long_msg(200, 'X');
        log.addMessage(long_msg);
        
        auto messages = log.getRecentMessages(1);
        REQUIRE(messages[0] == long_msg);
    }
}