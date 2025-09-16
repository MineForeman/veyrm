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
#include "log.h"

/**
 * @class MessageLogAdapter
 * @brief Adapts MessageLog to ILogger interface
 *
 * This adapter allows MessageLog to be used
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
        // Also log to debug file
        Log::combat(message);
    }

    void logSystem(const std::string& message) override {
        if (message_log) {
            message_log->addSystemMessage(message);
        }
    }

    // Debug logging methods - route to global Log system
    void logAI(const std::string& message) override {
        Log::ai(message);
    }

    void logTurn(const std::string& message) override {
        Log::turn(message);
    }

    void logMovement(const std::string& message) override {
        Log::movement(message);
    }

    void logInventory(const std::string& message) override {
        Log::inventory(message);
    }

    void logFOV(const std::string& message) override {
        Log::fov(message);
    }

    void logSpawn(const std::string& message) override {
        Log::spawn(message);
    }

    void logEnvironment(const std::string& message) override {
        Log::environment(message);
    }

private:
    MessageLog* message_log;
};