/**
 * @file log.h
 * @brief Global logging system for debug and development
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

/**
 * @class Log
 * @brief Global logging utility with categorized output
 *
 * The Log class provides a comprehensive logging system for debugging
 * and development. It supports multiple log levels, category-specific
 * logging, and both unified and separate log files for different
 * game systems.
 *
 * Features:
 * - Multiple log levels (ERROR, WARN, INFO, DEBUG, TRACE)
 * - Category-specific logging (combat, AI, movement, etc.)
 * - Separate log files per category for focused debugging
 * - Convenience macros for easy logging
 * - Runtime log level filtering
 *
 * Usage:
 * @code
 * Log::init("game.log", Log::DEBUG);
 * LOG_INFO("Game started");
 * LOG_COMBAT("Player attacks monster for 5 damage");
 * @endcode
 *
 * @see Config::getVerboseLogging()
 */
class Log {
public:
    /**
     * @enum Level
     * @brief Log message severity levels
     */
    enum Level {
        ERROR = 0,  ///< Critical errors that may cause crashes
        WARN = 1,   ///< Warning messages for potential issues
        INFO = 2,   ///< General information messages
        DEBUG = 3,  ///< Detailed debugging information
        TRACE = 4   ///< Extremely verbose tracing information
    };

    /**
     * @brief Initialize logging system
     * @param filename Main log file path (default: "debug.log")
     * @param level Minimum log level to output (default: INFO)
     */
    static void init(const std::string& filename = "debug.log", Level level = INFO);

    /**
     * @brief Shutdown logging system and close files
     * @note Called automatically at program exit
     */
    static void shutdown();

    // Standard log level methods

    /** @brief Log error message @param message Error text */
    static void error(const std::string& message);

    /** @brief Log warning message @param message Warning text */
    static void warn(const std::string& message);

    /** @brief Log info message @param message Information text */
    static void info(const std::string& message);

    /** @brief Log debug message @param message Debug text */
    static void debug(const std::string& message);

    /** @brief Log trace message @param message Trace text */
    static void trace(const std::string& message);

    // Category-specific logging methods

    /** @brief Log combat events @param message Combat action details */
    static void combat(const std::string& message);

    /** @brief Log AI decisions @param message AI behavior details */
    static void ai(const std::string& message);

    /** @brief Log turn system events @param message Turn processing details */
    static void turn(const std::string& message);

    /** @brief Log entity movement @param message Movement details */
    static void movement(const std::string& message);

    /** @brief Log player actions @param message Player action details */
    static void player(const std::string& message);

    /** @brief Log environment interactions @param message Door/trap/terrain events */
    static void environment(const std::string& message);

    /** @brief Log inventory operations @param message Item management details */
    static void inventory(const std::string& message);

    /** @brief Log monster spawning @param message Spawn event details */
    static void spawn(const std::string& message);

    /** @brief Log field of view updates @param message FOV calculation details */
    static void fov(const std::string& message);

    /** @brief Log map generation @param message Map creation details */
    static void map(const std::string& message);

    /** @brief Log UI events @param message UI state change details */
    static void ui(const std::string& message);

    /** @brief Log save/load operations @param message Serialization details */
    static void save(const std::string& message);

private:
    /**
     * @brief Internal logging method
     * @param level Message severity level
     * @param category Log category name
     * @param message Log message text
     */
    static void log(Level level, const std::string& category, const std::string& message);

    /**
     * @brief Convert log level to string
     * @param level Log level to convert
     * @return String representation of level
     */
    static std::string levelToString(Level level);

    /**
     * @brief Get log file stream for category
     * @param category Category name
     * @return Reference to appropriate file stream
     */
    static std::ofstream& getCategoryLogFile(const std::string& category);

    // File streams
    static std::ofstream logFile;           ///< Main unified log file
    static std::ofstream playerLogFile;     ///< Player actions log
    static std::ofstream envLogFile;        ///< Environment interactions log
    static std::ofstream combatLogFile;     ///< Combat events log
    static std::ofstream aiLogFile;         ///< AI decisions log
    static std::ofstream inventoryLogFile;  ///< Inventory operations log
    static std::ofstream mapLogFile;        ///< Map generation log
    static std::ofstream systemLogFile;     ///< System events log
    static std::ofstream turnLogFile;       ///< Turn processing log
    static std::ofstream fovLogFile;        ///< Field of view log
    static std::ofstream spawnLogFile;      ///< Monster spawning log

    static Level currentLevel;              ///< Current minimum log level
    static bool initialized;                ///< Whether logging is initialized
};

// Convenience macros for easier logging
#define LOG_ERROR(msg) Log::error(msg)
#define LOG_WARN(msg) Log::warn(msg)
#define LOG_INFO(msg) Log::info(msg)
#define LOG_DEBUG(msg) Log::debug(msg)
#define LOG_TRACE(msg) Log::trace(msg)

// Category-specific macros
#define LOG_COMBAT(msg) Log::combat(msg)
#define LOG_AI(msg) Log::ai(msg)
#define LOG_TURN(msg) Log::turn(msg)
#define LOG_MOVEMENT(msg) Log::movement(msg)
#define LOG_PLAYER(msg) Log::player(msg)
#define LOG_ENVIRONMENT(msg) Log::environment(msg)
#define LOG_INVENTORY(msg) Log::inventory(msg)
#define LOG_SPAWN(msg) Log::spawn(msg)
#define LOG_FOV(msg) Log::fov(msg)
#define LOG_MAP(msg) Log::map(msg)
#define LOG_UI(msg) Log::ui(msg)
#define LOG_SAVE(msg) Log::save(msg)