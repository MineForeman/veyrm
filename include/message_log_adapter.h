/**
 * @file message_log_adapter.h
 * @brief Adapter to make MessageLog work with ILogger interface
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

// All includes at global scope
#include "ecs/logger_interface.h"
#include "message_log.h"

/**
 * @class MessageLogAdapter
 * @brief Adapts MessageLog to ILogger interface
 *
 * This adapter allows the legacy MessageLog to be used
 * with ECS systems that expect an ILogger interface.
 */
class MessageLogAdapter : public ecs::ILogger {
public:
    explicit MessageLogAdapter(MessageLog* log) : message_log(log) {}

    void log(const std::string& message) override {
        if (message_log) {
            message_log->addMessage(message);
        }
    }

    void logCombat(const std::string& message) override {
        if (message_log) {
            message_log->addCombatMessage(message);
        }
    }

    void logSystem(const std::string& message) override {
        if (message_log) {
            message_log->addSystemMessage(message);
        }
    }

private:
    MessageLog* message_log;
};