/**
 * @file message_log.h
 * @brief In-game message log system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <deque>
#include <vector>
#include <memory>
#include <ftxui/dom/elements.hpp>

/**
 * @class MessageLog
 * @brief Manages in-game messages and notifications
 *
 * The MessageLog class handles all text messages shown to the player,
 * including combat results, system notifications, and other game events.
 * It maintains a rolling buffer of messages with automatic cleanup and
 * provides FTXUI rendering support for display in the game interface.
 *
 * Message categories:
 * - Regular messages: General game events and information
 * - Combat messages: Attack results, damage, death notifications
 * - System messages: Save/load, level changes, errors
 *
 * Features:
 * - Automatic message history management
 * - Recent message extraction for UI
 * - FTXUI rendering integration
 * - Configurable message buffer size
 *
 * @see Config::getMaxMessages()
 * @see Config::getVisibleMessages()
 * @see GameScreen
 */
class MessageLog {
public:
    /**
     * @brief Construct message log
     * @param max_messages Maximum number of messages to keep (default: 100)
     */
    MessageLog(size_t max_messages = 100);

    // Message addition methods

    /**
     * @brief Add a regular message
     * @param message Message text to add
     */
    void addMessage(const std::string& message);

    /**
     * @brief Add a combat-related message
     * @param message Combat event description
     * @note May use different formatting or color in future
     */
    void addCombatMessage(const std::string& message);

    /**
     * @brief Add a system notification message
     * @param message System event description
     * @note May use different formatting or color in future
     */
    void addSystemMessage(const std::string& message);

    // Message retrieval methods

    /**
     * @brief Get most recent messages for UI display
     * @param count Number of recent messages to return (default: 5)
     * @return Vector of recent message strings
     */
    std::vector<std::string> getRecentMessages(size_t count = 5) const;

    /**
     * @brief Get all messages (primarily for testing)
     * @return Vector of all stored messages
     */
    std::vector<std::string> getMessages() const;

    /**
     * @brief Render messages as FTXUI element
     * @param count Number of messages to render (default: 5)
     * @return FTXUI element for display in game interface
     */
    ftxui::Element render(size_t count = 5) const;

    /**
     * @brief Clear all messages
     * @note Removes all stored messages from the log
     */
    void clear();
    
private:
    std::deque<std::string> messages; ///< Rolling buffer of messages
    size_t max_size;                  ///< Maximum number of messages to store
};