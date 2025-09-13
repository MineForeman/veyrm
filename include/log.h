#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

// Global logging utility
class Log {
public:
    enum Level {
        ERROR = 0,
        WARN = 1,
        INFO = 2,
        DEBUG = 3,
        TRACE = 4
    };

    static void init(const std::string& filename = "debug.log", Level level = INFO);
    static void shutdown();

    static void error(const std::string& message);
    static void warn(const std::string& message);
    static void info(const std::string& message);
    static void debug(const std::string& message);
    static void trace(const std::string& message);

    // Category-specific logging methods
    static void combat(const std::string& message);      // Combat actions and damage
    static void ai(const std::string& message);          // Monster AI decisions
    static void turn(const std::string& message);        // Turn system events
    static void movement(const std::string& message);    // Entity movement
    static void player(const std::string& message);      // Player actions
    static void environment(const std::string& message); // Doors, traps, terrain
    static void inventory(const std::string& message);   // Item pickup/drop/use
    static void spawn(const std::string& message);       // Monster spawning
    static void fov(const std::string& message);         // Field of view updates
    static void map(const std::string& message);         // Map generation events
    static void ui(const std::string& message);          // UI state changes
    static void save(const std::string& message);        // Save/load operations

private:
    static void log(Level level, const std::string& category, const std::string& message);
    static std::string levelToString(Level level);
    static std::ofstream& getCategoryLogFile(const std::string& category);

    // Main debug log (contains everything)
    static std::ofstream logFile;

    // Separate log files for each category
    static std::ofstream playerLogFile;
    static std::ofstream envLogFile;
    static std::ofstream combatLogFile;
    static std::ofstream aiLogFile;
    static std::ofstream inventoryLogFile;
    static std::ofstream mapLogFile;
    static std::ofstream systemLogFile;
    static std::ofstream turnLogFile;
    static std::ofstream fovLogFile;
    static std::ofstream spawnLogFile;

    static Level currentLevel;
    static bool initialized;
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