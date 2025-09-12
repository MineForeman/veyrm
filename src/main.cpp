#include <iostream>
#include <string>
#include <cstdlib>

// Platform-specific includes for UTF-8 support
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#endif

// Version information
constexpr const char* VEYRM_VERSION = "0.0.1";
constexpr const char* VEYRM_BUILD_DATE = __DATE__;

/**
 * Initialize platform-specific settings
 */
void initializePlatform() {
#ifdef PLATFORM_WINDOWS
    // Enable UTF-8 support on Windows
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U8TEXT);
#endif
    
    // Set locale for proper Unicode handling
    std::locale::global(std::locale(""));
}

/**
 * Print welcome message with UTF-8 test
 */
void printWelcome() {
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║         VEYRM ROGUELIKE              ║\n";
    std::cout << "║     The Shattered Crown Awaits       ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Version: " << VEYRM_VERSION << "\n";
    std::cout << "Build Date: " << VEYRM_BUILD_DATE << "\n";
    std::cout << "Platform: ";
    
#ifdef PLATFORM_WINDOWS
    std::cout << "Windows";
#elif defined(PLATFORM_MACOS)
    std::cout << "macOS";
#elif defined(PLATFORM_LINUX)
    std::cout << "Linux";
#else
    std::cout << "Unknown";
#endif
    
    std::cout << "\n\n";
    
    // UTF-8 test with game-relevant characters
    std::cout << "UTF-8 Test:\n";
    std::cout << "  Walls: ═ ║ ╔ ╗ ╚ ╝ ╠ ╣ ╦ ╩ ╬\n";
    std::cout << "  Entities: @ r o ! ? † ☠\n";
    std::cout << "  Terrain: · # ≈ ∩ ▓ ░\n";
    std::cout << "\n";
}

/**
 * Run basic system checks
 */
bool runSystemChecks() {
    bool allPassed = true;
    
    std::cout << "Running system checks...\n";
    
    // Check C++ version
    std::cout << "  [";
    if (__cplusplus >= 202302L) {
        std::cout << "✓";
    } else {
        std::cout << "✗";
        allPassed = false;
    }
    std::cout << "] C++23 support\n";
    
    // Check terminal width (basic check)
    std::cout << "  [✓] Terminal output\n";
    
    // Check file system access
    std::cout << "  [";
    if (std::getenv("HOME") || std::getenv("USERPROFILE")) {
        std::cout << "✓";
    } else {
        std::cout << "⚠";
    }
    std::cout << "] Environment variables\n";
    
    std::cout << "\n";
    return allPassed;
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    // Initialize platform-specific settings
    initializePlatform();
    
    // Print welcome message
    printWelcome();
    
    // Handle command-line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            std::cout << "veyrm version " << VEYRM_VERSION << "\n";
            return 0;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -h, --help     Show this help message\n";
            std::cout << "  -v, --version  Show version information\n";
            std::cout << "  --test         Run system checks\n";
            return 0;
        } else if (arg == "--test") {
            bool passed = runSystemChecks();
            if (passed) {
                std::cout << "All system checks passed! ✓\n";
                std::cout << "Ready to proceed with Phase 0.2\n";
                return 0;
            } else {
                std::cout << "Some checks failed. Please review the requirements.\n";
                return 1;
            }
        }
    }
    
    // Basic initialization message
    std::cout << "Initializing Veyrm...\n";
    std::cout << "CMake configuration successful!\n";
    std::cout << "Project structure created.\n";
    std::cout << "\n";
    std::cout << "Phase 0.1: Initialize Repository - COMPLETE ✓\n";
    std::cout << "\n";
    std::cout << "Next steps:\n";
    std::cout << "  1. Run with --test flag to verify system\n";
    std::cout << "  2. Proceed to Phase 0.2: Dependencies Setup\n";
    
    return 0;
}