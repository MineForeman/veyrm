#include "message_log.h"

using namespace ftxui;

MessageLog::MessageLog(size_t max_messages) 
    : max_size(max_messages) {
    addMessage("Welcome to Veyrm!");
    addMessage("You descend into the Spiral Vaults...");
}

void MessageLog::addMessage(const std::string& message) {
    messages.push_back(message);
    while (messages.size() > max_size) {
        messages.pop_front();
    }
}

void MessageLog::addCombatMessage(const std::string& message) {
    addMessage("[Combat] " + message);
}

void MessageLog::addSystemMessage(const std::string& message) {
    addMessage("[System] " + message);
}

std::vector<std::string> MessageLog::getRecentMessages(size_t count) const {
    std::vector<std::string> recent;
    size_t start = messages.size() > count ? messages.size() - count : 0;

    for (size_t i = start; i < messages.size(); ++i) {
        recent.push_back(messages[i]);
    }

    return recent;
}

std::vector<std::string> MessageLog::getMessages() const {
    std::vector<std::string> all_messages;
    for (const auto& msg : messages) {
        all_messages.push_back(msg);
    }
    return all_messages;
}

Element MessageLog::render(size_t count) const {
    std::vector<Element> elements;
    auto recent = getRecentMessages(count);
    
    for (const auto& msg : recent) {
        elements.push_back(text(msg));
    }
    
    // Pad with empty lines if needed
    while (elements.size() < count) {
        elements.push_back(text(""));
    }
    
    return vbox(elements);
}

void MessageLog::clear() {
    messages.clear();
    addMessage("Message log cleared.");
}