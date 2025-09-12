#pragma once

#include <string>
#include <queue>
#include <ftxui/component/event.hpp>

class TestInput {
public:
    TestInput();
    
    // Load keystrokes from a string
    void loadKeystrokes(const std::string& keystrokes);
    
    // Check if there are more keystrokes
    bool hasNextKeystroke() const;
    
    // Get the next keystroke as an FTXUI Event
    ftxui::Event getNextKeystroke();
    
    // Clear all keystrokes
    void clear();
    
    // Get remaining keystroke count
    size_t remainingCount() const;
    
private:
    std::queue<ftxui::Event> events;
    
    // Convert a character to an FTXUI Event
    ftxui::Event charToEvent(char c);
};