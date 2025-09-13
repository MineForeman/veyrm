#include "log.h"
#include <chrono>
#include <iomanip>

// Static member definitions
std::ofstream Log::logFile;
Log::Level Log::currentLevel = Log::INFO;
bool Log::initialized = false;

void Log::init(const std::string& filename, Level level) {
    if (initialized) {
        shutdown();
    }

    logFile.open(filename, std::ios::app);
    currentLevel = level;
    initialized = true;

    // Log initialization
    log(INFO, "SYSTEM", "=== Debug logging initialized ===");
    log(INFO, "SYSTEM", "Log level: " + levelToString(currentLevel));
}

void Log::shutdown() {
    if (initialized && logFile.is_open()) {
        log(INFO, "SYSTEM", "=== Debug logging shutdown ===");
        logFile.close();
    }
    initialized = false;
}

void Log::error(const std::string& message) {
    log(ERROR, "ERROR", message);
}

void Log::warn(const std::string& message) {
    log(WARN, "WARN", message);
}

void Log::info(const std::string& message) {
    log(INFO, "INFO", message);
}

void Log::debug(const std::string& message) {
    log(DEBUG, "DEBUG", message);
}

void Log::trace(const std::string& message) {
    log(TRACE, "TRACE", message);
}

void Log::combat(const std::string& message) {
    log(DEBUG, "COMBAT", message);
}

void Log::ai(const std::string& message) {
    log(DEBUG, "AI", message);
}

void Log::turn(const std::string& message) {
    log(DEBUG, "TURN", message);
}

void Log::movement(const std::string& message) {
    log(DEBUG, "MOVE", message);
}

void Log::log(Level level, const std::string& category, const std::string& message) {
    if (!initialized || level > currentLevel) {
        return;
    }

    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    ss << " [" << std::setw(6) << std::left << category << "] " << message;

    std::string logLine = ss.str();

    // Write to file
    if (logFile.is_open()) {
        logFile << logLine << std::endl;
        logFile.flush();
    }

    // Don't output to console during normal gameplay - it interferes with the display
    // Only output errors to console
    if (level == ERROR) {
        std::cerr << logLine << std::endl;
    }
}

std::string Log::levelToString(Level level) {
    switch (level) {
        case ERROR: return "ERROR";
        case WARN: return "WARN";
        case INFO: return "INFO";
        case DEBUG: return "DEBUG";
        case TRACE: return "TRACE";
        default: return "UNKNOWN";
    }
}