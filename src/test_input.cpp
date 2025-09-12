#include "test_input.h"
#include <iostream>

using namespace ftxui;

TestInput::TestInput() {}

void TestInput::loadKeystrokes(const std::string& keystrokes) {
    clear();
    
    for (size_t i = 0; i < keystrokes.length(); ++i) {
        char c = keystrokes[i];
        
        // Handle escape sequences
        if (c == '\\' && i + 1 < keystrokes.length()) {
            char next = keystrokes[i + 1];
            switch (next) {
                case 'n':  // Enter key
                    events.push(Event::Return);
                    ++i;
                    break;
                case 'e':  // Escape key
                    events.push(Event::Escape);
                    ++i;
                    break;
                case 'u':  // Up arrow
                    events.push(Event::ArrowUp);
                    ++i;
                    break;
                case 'd':  // Down arrow
                    events.push(Event::ArrowDown);
                    ++i;
                    break;
                case 'l':  // Left arrow
                    events.push(Event::ArrowLeft);
                    ++i;
                    break;
                case 'r':  // Right arrow
                    events.push(Event::ArrowRight);
                    ++i;
                    break;
                case 't':  // Tab
                    events.push(Event::Tab);
                    ++i;
                    break;
                case 'b':  // Backspace
                    events.push(Event::Backspace);
                    ++i;
                    break;
                case '\\':  // Literal backslash
                    events.push(Event::Character('\\'));
                    ++i;
                    break;
                default:
                    // Unknown escape sequence, treat as literal
                    events.push(Event::Character(c));
                    break;
            }
        } else {
            // Regular character
            events.push(charToEvent(c));
        }
    }
    
    std::cout << "Loaded " << events.size() << " test keystrokes\n";
}

bool TestInput::hasNextKeystroke() const {
    return !events.empty();
}

Event TestInput::getNextKeystroke() {
    if (events.empty()) {
        return Event::Special("empty");
    }
    
    Event e = events.front();
    events.pop();
    return e;
}

void TestInput::clear() {
    while (!events.empty()) {
        events.pop();
    }
}

size_t TestInput::remainingCount() const {
    return events.size();
}

Event TestInput::charToEvent(char c) {
    // Special handling for common control characters
    switch (c) {
        case '\n':
        case '\r':
            return Event::Return;
        case '\t':
            return Event::Tab;
        case 127:
        case '\b':
            return Event::Backspace;
        case 27:
            return Event::Escape;
        default:
            return Event::Character(c);
    }
}