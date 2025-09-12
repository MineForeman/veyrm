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
    
    // Enable/disable frame dumping
    void setFrameDumpMode(bool enabled) { frame_dump_mode = enabled; }
    bool isFrameDumpMode() const { return frame_dump_mode; }
    
private:
    std::queue<ftxui::Event> events;
    bool frame_dump_mode = false;
    
    // Convert a character to an FTXUI Event
    ftxui::Event charToEvent(char c);
};