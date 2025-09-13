#pragma once

#include <string>
#include <deque>
#include <vector>
#include <ftxui/dom/elements.hpp>

class MessageLog {
public:
    MessageLog(size_t max_messages = 100);
    
    // Add messages
    void addMessage(const std::string& message);
    void addCombatMessage(const std::string& message);
    void addSystemMessage(const std::string& message);
    
    // Get messages for display
    std::vector<std::string> getRecentMessages(size_t count = 5) const;
    std::vector<std::string> getMessages() const;  // For testing
    ftxui::Element render(size_t count = 5) const;
    
    // Clear
    void clear();
    
private:
    std::deque<std::string> messages;
    size_t max_size;
};