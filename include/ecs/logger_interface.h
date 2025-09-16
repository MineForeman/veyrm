/**
 * @file logger_interface.h
 * @brief Abstract logger interface for ECS systems
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <functional>

namespace ecs {

/**
 * @class ILogger
 * @brief Abstract interface for logging messages
 *
 * This interface allows ECS systems to log messages without
 * depending on specific UI implementations like FTXUI.
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    /**
     * @brief Log a general message
     * @param message Message to log
     */
    virtual void log(const std::string& message) = 0;

    /**
     * @brief Log a combat message
     * @param message Combat message
     */
    virtual void logCombat(const std::string& message) = 0;

    /**
     * @brief Log a system message
     * @param message System message
     */
    virtual void logSystem(const std::string& message) = 0;

    /**
     * @brief Log an error message
     * @param message Error message
     */
    virtual void logError(const std::string& message) { log("[ERROR] " + message); }

    /**
     * @brief Log a warning message
     * @param message Warning message
     */
    virtual void logWarning(const std::string& message) { log("[WARNING] " + message); }

    // Debug logging methods for different categories
    /**
     * @brief Log AI debug information
     * @param message AI debug message
     */
    virtual void logAI([[maybe_unused]] const std::string& message) {}

    /**
     * @brief Log turn system debug information
     * @param message Turn debug message
     */
    virtual void logTurn([[maybe_unused]] const std::string& message) {}

    /**
     * @brief Log movement debug information
     * @param message Movement debug message
     */
    virtual void logMovement([[maybe_unused]] const std::string& message) {}

    /**
     * @brief Log inventory debug information
     * @param message Inventory debug message
     */
    virtual void logInventory([[maybe_unused]] const std::string& message) {}

    /**
     * @brief Log field of view debug information
     * @param message FOV debug message
     */
    virtual void logFOV([[maybe_unused]] const std::string& message) {}

    /**
     * @brief Log spawning debug information
     * @param message Spawn debug message
     */
    virtual void logSpawn([[maybe_unused]] const std::string& message) {}

    /**
     * @brief Log environment debug information
     * @param message Environment debug message
     */
    virtual void logEnvironment([[maybe_unused]] const std::string& message) {}
};

/**
 * @brief Simple function-based logger
 *
 * For systems that just need a simple callback
 */
using LogCallback = std::function<void(const std::string&)>;

} // namespace ecs