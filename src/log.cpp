#include "log.h"
#include <chrono>
#include <iomanip>
#include <filesystem>

// Static member definitions
std::ofstream Log::logFile;
std::ofstream Log::playerLogFile;
std::ofstream Log::envLogFile;
std::ofstream Log::combatLogFile;
std::ofstream Log::aiLogFile;
std::ofstream Log::inventoryLogFile;
std::ofstream Log::mapLogFile;
std::ofstream Log::systemLogFile;
std::ofstream Log::turnLogFile;
std::ofstream Log::fovLogFile;
std::ofstream Log::spawnLogFile;
Log::Level Log::currentLevel = Log::INFO;
bool Log::initialized = false;

void Log::init(const std::string& filename, Level level) {
    if (initialized) {
        shutdown();
    }

    // Open main debug log (contains everything)
    logFile.open(filename, std::ios::app);

    // Create logs directory if it doesn't exist
    std::string logDir = "logs/";
    std::filesystem::create_directories(logDir);
    playerLogFile.open(logDir + "veyrm_player.log", std::ios::app);
    envLogFile.open(logDir + "veyrm_env.log", std::ios::app);
    combatLogFile.open(logDir + "veyrm_combat.log", std::ios::app);
    aiLogFile.open(logDir + "veyrm_ai.log", std::ios::app);
    inventoryLogFile.open(logDir + "veyrm_inventory.log", std::ios::app);
    mapLogFile.open(logDir + "veyrm_map.log", std::ios::app);
    systemLogFile.open(logDir + "veyrm_system.log", std::ios::app);
    turnLogFile.open(logDir + "veyrm_turn.log", std::ios::app);
    fovLogFile.open(logDir + "veyrm_fov.log", std::ios::app);
    spawnLogFile.open(logDir + "veyrm_spawn.log", std::ios::app);

    currentLevel = level;
    initialized = true;

    // Log initialization
    log(INFO, "SYSTEM", "=== Debug logging initialized ===");
    log(INFO, "SYSTEM", "Log level: " + levelToString(currentLevel));
}

void Log::shutdown() {
    if (initialized) {
        log(INFO, "SYSTEM", "=== Debug logging shutdown ===");

        // Close all log files
        if (logFile.is_open()) logFile.close();
        if (playerLogFile.is_open()) playerLogFile.close();
        if (envLogFile.is_open()) envLogFile.close();
        if (combatLogFile.is_open()) combatLogFile.close();
        if (aiLogFile.is_open()) aiLogFile.close();
        if (inventoryLogFile.is_open()) inventoryLogFile.close();
        if (mapLogFile.is_open()) mapLogFile.close();
        if (systemLogFile.is_open()) systemLogFile.close();
        if (turnLogFile.is_open()) turnLogFile.close();
        if (fovLogFile.is_open()) fovLogFile.close();
        if (spawnLogFile.is_open()) spawnLogFile.close();
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

void Log::player(const std::string& message) {
    log(DEBUG, "PLAYER", message);
}

void Log::environment(const std::string& message) {
    log(INFO, "ENV", message);
}

void Log::inventory(const std::string& message) {
    log(DEBUG, "INV", message);
}

void Log::spawn(const std::string& message) {
    log(DEBUG, "SPAWN", message);
}

void Log::fov(const std::string& message) {
    log(TRACE, "FOV", message);
}

void Log::map(const std::string& message) {
    log(INFO, "MAP", message);
}

void Log::ui(const std::string& message) {
    log(DEBUG, "UI", message);
}

void Log::save(const std::string& message) {
    log(INFO, "SAVE", message);
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

    // Write to main debug file (contains everything)
    if (logFile.is_open()) {
        logFile << logLine << std::endl;
        logFile.flush();
    }

    // Also write to category-specific file
    std::ofstream& categoryFile = getCategoryLogFile(category);
    if (categoryFile.is_open()) {
        categoryFile << logLine << std::endl;
        categoryFile.flush();
    }

    // Don't output to console during normal gameplay - it interferes with the display
    // Only output errors to console
    if (level == ERROR) {
        std::cerr << logLine << std::endl;
    }
}

std::ofstream& Log::getCategoryLogFile(const std::string& category) {
    if (category == "PLAYER") return playerLogFile;
    if (category == "ENV") return envLogFile;
    if (category == "COMBAT") return combatLogFile;
    if (category == "AI") return aiLogFile;
    if (category == "INV") return inventoryLogFile;
    if (category == "MAP") return mapLogFile;
    if (category == "SYSTEM" || category == "ERROR" || category == "WARN" ||
        category == "INFO" || category == "DEBUG" || category == "TRACE") return systemLogFile;
    if (category == "TURN") return turnLogFile;
    if (category == "FOV") return fovLogFile;
    if (category == "SPAWN") return spawnLogFile;
    if (category == "MOVE") return aiLogFile; // Monster movement goes to AI log
    if (category == "UI") return systemLogFile; // UI goes to system log
    if (category == "SAVE") return systemLogFile; // Save goes to system log

    // Default to system log for unknown categories
    return systemLogFile;
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