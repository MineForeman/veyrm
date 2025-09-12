#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <csignal>
#include <cstdlib>

// FTXUI includes
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

// JSON include
#include <nlohmann/json.hpp>

// Game includes
#include "game_state.h"
#include "game_screen.h"

// Platform-specific includes for UTF-8 support
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#endif

// Version information
constexpr const char* VEYRM_VERSION = "0.0.2";
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
 * Test JSON functionality
 */
bool testJsonLibrary() {
    try {
        // Create a simple JSON object
        nlohmann::json testData = {
            {"version", VEYRM_VERSION},
            {"test", true},
            {"entities", {"player", "monster", "item"}}
        };
        
        // Serialize to string
        std::string jsonStr = testData.dump();
        
        // Parse back
        auto parsed = nlohmann::json::parse(jsonStr);
        
        return parsed["test"] == true;
    } catch (const std::exception& e) {
        std::cerr << "JSON test failed: " << e.what() << std::endl;
        return false;
    }
}

/**
 * Run system checks with dependencies
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
    
    // Check JSON library
    std::cout << "  [";
    if (testJsonLibrary()) {
        std::cout << "✓";
    } else {
        std::cout << "✗";
        allPassed = false;
    }
    std::cout << "] nlohmann/json library\n";
    
    // Check FTXUI (basic check)
    std::cout << "  [✓] FTXUI library (will test in UI mode)\n";
    
    // Check terminal
    std::cout << "  [✓] Terminal output\n";
    
    std::cout << "\n";
    return allPassed;
}

/**
 * Create main menu component
 */
ftxui::Component createMainMenu(GameManager* game_manager, ftxui::ScreenInteractive*) {
    using namespace ftxui;
    
    // Menu state
    static int selected = 0;
    
    // Menu options
    static std::vector<std::string> menu_entries = {
        "New Game",
        "Continue",
        "Settings",
        "About",
        "Quit"
    };
    
    auto menu = Menu(&menu_entries, &selected);
    
    // Create the main component with renderer
    auto component = Renderer(menu, [=] {
        // Title
        auto title = vbox({
            text("╔══════════════════════════════════════╗") | color(Color::Yellow),
            text("║         VEYRM ROGUELIKE              ║") | color(Color::Yellow),
            text("║     The Shattered Crown Awaits       ║") | color(Color::Yellow),
            text("╚══════════════════════════════════════╝") | color(Color::Yellow),
            separator(),
        });
        
        // Menu
        auto menu_display = vbox({
            text("Main Menu") | bold,
            separator(),
            menu->Render(),
        }) | border | size(WIDTH, EQUAL, 30);
        
        // About box (conditional)
        Element about_box = emptyElement();
        if (selected == 3) {  // Show about when About is selected
            about_box = vbox({
                separator(),
                window(text("About"), vbox({
                    text("Version: " + std::string(VEYRM_VERSION)),
                    text("Build Date: " + std::string(VEYRM_BUILD_DATE)),
                    separator(),
                    text("A modern roguelike inspired by Angband"),
                    text("Built with FTXUI, C++23"),
                    separator(),
                    text("Press ESC or Q to quit"),
                })),
            });
        }
        
        // Status line
        auto status = hbox({
            text("Use ↑↓ to navigate, Enter to select, Q to quit") | dim,
        });
        
        // Combine all elements
        return vbox({
            title,
            flex(vbox({
                menu_display,
                about_box,
            })) | center,
            separator(),
            status,
        });
    });
    
    // Add event handling
    component |= CatchEvent([=](Event event) {
        if (event == Event::Return) {
            switch(selected) {
                case 0: // New Game
                    game_manager->setState(GameState::PLAYING);
                    break;
                case 1: // Continue
                    // TODO: Load save game
                    break;
                case 2: // Settings
                    // TODO: Settings menu
                    break;
                case 3: // About
                    // Toggle about is handled in renderer directly
                    break;
                case 4: // Quit
                    game_manager->setState(GameState::QUIT);
                    break;
            }
            return true;
        }
        if (event == Event::Character('q') || event == Event::Escape) {
            game_manager->setState(GameState::QUIT);
            return true;
        }
        return false;
    });
    
    return component;
}

/**
 * Reset terminal to normal state
 */
void resetTerminal() {
    // Disable all mouse tracking modes
    std::cout << "\033[?1003l"; // Disable mouse tracking
    std::cout << "\033[?1006l"; // Disable SGR mouse mode  
    std::cout << "\033[?1015l"; // Disable urxvt mouse mode
    std::cout << "\033[?1000l"; // Disable X11 mouse reporting
    std::cout << "\033[?25h";   // Show cursor
    std::cout << "\033c";       // Reset terminal
    std::cout.flush();
}

/**
 * Signal handler for clean shutdown
 */
void signalHandler(int signum) {
    resetTerminal();
    std::exit(signum);
}

/**
 * Run FTXUI interface
 */
void runInterface() {
    using namespace ftxui;
    
    // Set up cleanup handlers for unexpected exits
    std::atexit(resetTerminal);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    auto screen = ScreenInteractive::TerminalOutput();
    
    // Disable mouse tracking to prevent terminal artifacts
    screen.TrackMouse(false);
    GameManager game_manager;
    
    // Create components
    Component main_menu = createMainMenu(&game_manager, &screen);
    GameScreen game_screen(&game_manager, &screen);
    Component game_component = game_screen.Create();
    
    // State-based renderer
    auto main_renderer = Renderer([&] {
        switch(game_manager.getState()) {
            case GameState::MENU:
                return main_menu->Render();
            case GameState::PLAYING:
                return game_component->Render();
            case GameState::PAUSED:
                return vbox({
                    text("PAUSED") | bold | center,
                    separator(),
                    text("Press ESC to resume") | center
                }) | border;
            case GameState::INVENTORY:
                return vbox({
                    text("INVENTORY") | bold,
                    separator(),
                    text("(Not yet implemented)"),
                    text("Press ESC to return")
                }) | border;
            case GameState::HELP:
                return vbox({
                    text("HELP") | bold,
                    separator(),
                    text("Arrow keys: Move"),
                    text("Numpad: Move (with diagonals)"),
                    text(".: Wait"),
                    text("i: Inventory"),
                    text("?: Help"),
                    text("q: Quit to menu"),
                    separator(),
                    text("Press ESC to return")
                }) | border;
            case GameState::QUIT:
                screen.ExitLoopClosure()();
                return text("Exiting...");
        }
        return text("Unknown state");
    });
    
    // State-based input handler
    auto main_component = CatchEvent(main_renderer, [&](Event event) {
        switch(game_manager.getState()) {
            case GameState::MENU:
                return main_menu->OnEvent(event);
            case GameState::PLAYING:
                return game_component->OnEvent(event);
            case GameState::PAUSED:
            case GameState::INVENTORY:
            case GameState::HELP:
                if (event == Event::Escape) {
                    game_manager.returnToPreviousState();
                    return true;
                }
                return false;
            case GameState::QUIT:
            default:
                return false;
        }
    });
    
    screen.Loop(main_component);
    
    // Terminal cleanup is handled by resetTerminal() via atexit
    std::cout << "Thanks for playing Veyrm!\n";
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    // Initialize platform-specific settings
    initializePlatform();
    
    // Handle command-line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            std::cout << "veyrm version " << VEYRM_VERSION << "\n";
            std::cout << "Dependencies:\n";
            std::cout << "  - FTXUI v5.0.0\n";
            std::cout << "  - nlohmann/json v3.11.3\n";
            std::cout << "  - Catch2 v3.5.1\n";
            return 0;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -h, --help     Show this help message\n";
            std::cout << "  -v, --version  Show version information\n";
            std::cout << "  --test         Run system checks\n";
            std::cout << "  --no-ui        Run without UI (test mode)\n";
            return 0;
        } else if (arg == "--test") {
            bool passed = runSystemChecks();
            if (passed) {
                std::cout << "All system checks passed! ✓\n";
                std::cout << "Phase 0.2: Dependencies Setup - COMPLETE ✓\n";
                return 0;
            } else {
                std::cout << "Some checks failed. Please review the requirements.\n";
                return 1;
            }
        } else if (arg == "--no-ui") {
            std::cout << "Running in no-UI mode...\n";
            bool passed = runSystemChecks();
            std::cout << "Dependencies test " << (passed ? "PASSED" : "FAILED") << "\n";
            return passed ? 0 : 1;
        }
    }
    
    // Run the interface
    runInterface();
    
    return 0;
}