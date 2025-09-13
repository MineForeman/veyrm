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

    // Combat-specific logging
    static void combat(const std::string& message);

    // AI-specific logging
    static void ai(const std::string& message);

    // Turn system logging
    static void turn(const std::string& message);

    // Movement logging
    static void movement(const std::string& message);

private:
    static void log(Level level, const std::string& category, const std::string& message);
    static std::string levelToString(Level level);

    static std::ofstream logFile;
    static Level currentLevel;
    static bool initialized;
};

// Convenience macros for easier logging
#define LOG_ERROR(msg) Log::error(msg)
#define LOG_WARN(msg) Log::warn(msg)
#define LOG_INFO(msg) Log::info(msg)
#define LOG_DEBUG(msg) Log::debug(msg)
#define LOG_TRACE(msg) Log::trace(msg)
#define LOG_COMBAT(msg) Log::combat(msg)
#define LOG_AI(msg) Log::ai(msg)
#define LOG_TURN(msg) Log::turn(msg)
#define LOG_MOVEMENT(msg) Log::movement(msg)