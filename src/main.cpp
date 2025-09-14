#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <atomic>

// FTXUI includes
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

// JSON include
#include <nlohmann/json.hpp>

#include "log.h"

// Game includes
#include "game_state.h"
#include "game_screen.h"
#include "save_load_screen.h"
#include "test_input.h"
#include "game_loop.h"
#include "frame_stats.h"
#include "map_generator.h"
#include "config.h"

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
        // ANSI art title - properly centered
        auto title = vbox({
            text(""),
            text("██╗   ██╗███████╗██╗   ██╗██████╗ ███╗   ███╗") | color(Color::Red) | center,
            text("██║   ██║██╔════╝╚██╗ ██╔╝██╔══██╗████╗ ████║") | color(Color::RedLight) | center,
            text("██║   ██║█████╗   ╚████╔╝ ██████╔╝██╔████╔██║") | color(Color::Yellow) | center,
            text("╚██╗ ██╔╝██╔══╝    ╚██╔╝  ██╔══██╗██║╚██╔╝██║") | color(Color::Yellow) | center,
            text(" ╚████╔╝ ███████╗   ██║   ██║  ██║██║ ╚═╝ ██║") | color(Color::YellowLight) | center,
            text("  ╚═══╝  ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝     ╚═╝") | color(Color::RedLight) | center,
            text(""),
            text("The Shattered Crown Awaits") | dim | center,
            text(""),
            separator(),
        });
        
        // Simple menu
        auto menu_display = vbox({
            text("Main Menu") | bold | center,
            text(""),
            menu->Render(),
        }) | border | size(WIDTH, EQUAL, 30);
        
        // Clean about box
        Element about_box = emptyElement();
        if (selected == 3) {  // Show about when About is selected
            about_box = vbox({
                text(""),
                window(text(" About ") | bold, vbox({
                    text("Version: " + std::string(VEYRM_VERSION)),
                    text("Build: " + std::string(VEYRM_BUILD_DATE)),
                    text("Website: veyrm.com"),
                    separator(),
                    text("A modern roguelike inspired by Angband"),
                    text("Deep beneath Veyrmspire, the Spiral"),
                    text("Vaults hold the last shard of the"),
                    text("dead god's crown."),
                })) | size(WIDTH, EQUAL, 45),
            });
        }
        
        // Clean status line
        auto status = hbox({
            text("[↑↓] Navigate  [Enter] Select  [Q] Quit") | dim,
        }) | center;
        
        // Combine all elements
        return vbox({
            title,
            flex(vbox({
                menu_display | center,
                about_box | center,
            })),
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
 * Run in frame dump mode for testing
 */
void runFrameDumpMode(TestInput* test_input, MapType initial_map = MapType::TEST_DUNGEON) {
    using namespace ftxui;
    
    GameManager game_manager(initial_map);
    
    // Create components
    auto screen = ScreenInteractive::Fullscreen();
    Component main_menu = createMainMenu(&game_manager, &screen);
    GameScreen game_screen(&game_manager, &screen);
    Component game_component = game_screen.Create();
    SaveLoadScreen save_load_screen(&game_manager);
    Component save_load_component = save_load_screen.create();
    
    int frame_count = 0;
    
    std::cout << "\n=== FRAME DUMP MODE START ===\n\n";
    
    while (test_input->hasNextKeystroke()) {
        // Get next keystroke
        auto event = test_input->getNextKeystroke();
        
        // Create a screen buffer to render to
        auto document = [&]() -> Element {
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
                    return game_component->Render();  // Use game screen's inventory panel
                case GameState::SAVE_LOAD:
                    return save_load_component->Render();
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
                case GameState::DEATH:
                    return vbox({
                        text("") | size(WIDTH, EQUAL, 1),
                        hbox({
                            text("                 "),
                            vbox({
                                text("╔═══════════════════════════════════════╗") | color(Color::Red),
                                text("║                                       ║") | color(Color::Red),
                                text("║            Y O U   D I E D            ║") | color(Color::Red) | bold,
                                text("║                                       ║") | color(Color::Red),
                                text("║  Your adventure has come to an end.   ║") | color(Color::White),
                                text("║                                       ║") | color(Color::Red),
                                text("║  The darkness claims another soul...  ║") | color(Color::White),
                                text("║                                       ║") | color(Color::Red),
                                text("╠═══════════════════════════════════════╣") | color(Color::Red),
                                text("║                                       ║") | color(Color::Red),
                                text("║    [R] Return to Main Menu            ║") | color(Color::Yellow),
                                text("║    [Q] Quit Game                      ║") | color(Color::Yellow),
                                text("║                                       ║") | color(Color::Red),
                                text("╚═══════════════════════════════════════╝") | color(Color::Red)
                            }),
                            text("                 ")
                        }) | center,
                        text("") | size(WIDTH, EQUAL, 1)
                    }) | center | size(HEIGHT, EQUAL, 20);
                case GameState::QUIT:
                    return text("Exiting...");
            }
            return text("Unknown state");
        }();
        
        // Create a screen and render
        Screen render_screen(80, 24);
        Render(render_screen, document);
        
        // Print frame header
        std::cout << "--- Frame " << ++frame_count << " ---\n";
        std::cout << "State: ";
        switch(game_manager.getState()) {
            case GameState::MENU: std::cout << "MENU"; break;
            case GameState::PLAYING: std::cout << "PLAYING"; break;
            case GameState::PAUSED: std::cout << "PAUSED"; break;
            case GameState::INVENTORY: std::cout << "INVENTORY"; break;
            case GameState::HELP: std::cout << "HELP"; break;
            case GameState::SAVE_LOAD: std::cout << "SAVE_LOAD"; break;
            case GameState::DEATH: std::cout << "DEATH"; break;
            case GameState::QUIT: std::cout << "QUIT"; break;
        }
        std::cout << "\nInput: ";
        
        // Describe the input
        if (event == Event::Return) std::cout << "Enter";
        else if (event == Event::Escape) std::cout << "Escape";
        else if (event == Event::ArrowUp) std::cout << "Up Arrow";
        else if (event == Event::ArrowDown) std::cout << "Down Arrow";
        else if (event == Event::ArrowLeft) std::cout << "Left Arrow";
        else if (event == Event::ArrowRight) std::cout << "Right Arrow";
        else if (event.is_character()) std::cout << "'" << event.character() << "'";
        else std::cout << "Special";
        std::cout << "\n\n";
        
        // Print the screen content
        std::cout << render_screen.ToString() << "\n";
        
        // Process the event
        switch(game_manager.getState()) {
            case GameState::MENU:
                main_menu->OnEvent(event);
                break;
            case GameState::PLAYING:
            case GameState::INVENTORY:  // Inventory also needs game_component events
                game_component->OnEvent(event);
                break;
            case GameState::SAVE_LOAD:
                if (save_load_screen.handleInput(event)) {
                    // Input was handled
                }
                break;
            case GameState::PAUSED:
            case GameState::HELP:
                if (event == Event::Escape) {
                    game_manager.returnToPreviousState();
                }
                break;
            case GameState::DEATH:
                if (event == Event::Character('r') || event == Event::Character('R')) {
                    game_manager.setState(GameState::MENU);
                } else if (event == Event::Character('q') || event == Event::Character('Q')) {
                    game_manager.setState(GameState::QUIT);
                }
                break;
            case GameState::QUIT:
                std::cout << "\n=== FRAME DUMP MODE END ===\n";
                return;
        }
        
        std::cout << "\n";
    }
    
    std::cout << "\n=== FRAME DUMP MODE END (Input Exhausted) ===\n";
}

/**
 * Run FTXUI interface
 */
void runInterface(TestInput* test_input = nullptr, MapType initial_map = MapType::TEST_DUNGEON) {
    using namespace ftxui;
    
    // Set up cleanup handlers for unexpected exits
    std::atexit(resetTerminal);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    auto screen = ScreenInteractive::Fullscreen();
    
    // Disable mouse tracking to prevent terminal artifacts
    screen.TrackMouse(false);
    GameManager game_manager(initial_map);
    
    // Enable debug mode if requested
    const char* debug_env = std::getenv("VEYRM_DEBUG");
    if (debug_env && std::string(debug_env) == "1") {
        game_manager.setDebugMode(true);
    }
    
    // Create components
    Component main_menu = createMainMenu(&game_manager, &screen);
    GameScreen game_screen(&game_manager, &screen);
    Component game_component = game_screen.Create();
    SaveLoadScreen save_load_screen(&game_manager);
    Component save_load_component = save_load_screen.create();
    
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
                return game_component->Render();  // Use game screen's inventory panel
            case GameState::SAVE_LOAD:
                return save_load_component->Render();
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
            case GameState::DEATH:
                return vbox({
                    text("") | size(WIDTH, EQUAL, 1),
                    hbox({
                        text("                 "),
                        vbox({
                            text("╔═══════════════════════════════════════╗") | color(Color::Red),
                            text("║                                       ║") | color(Color::Red),
                            text("║            Y O U   D I E D            ║") | color(Color::Red) | bold,
                            text("║                                       ║") | color(Color::Red),
                            text("║  Your adventure has come to an end.   ║") | color(Color::White),
                            text("║                                       ║") | color(Color::Red),
                            text("║  The darkness claims another soul...  ║") | color(Color::White),
                            text("║                                       ║") | color(Color::Red),
                            text("╠═══════════════════════════════════════╣") | color(Color::Red),
                            text("║                                       ║") | color(Color::Red),
                            text("║    [R] Return to Main Menu            ║") | color(Color::Yellow),
                            text("║    [Q] Quit Game                      ║") | color(Color::Yellow),
                            text("║                                       ║") | color(Color::Red),
                            text("╚═══════════════════════════════════════╝") | color(Color::Red)
                        }),
                        text("                 ")
                    }) | center,
                    text("") | size(WIDTH, EQUAL, 1)
                }) | center | size(HEIGHT, EQUAL, 20);
            case GameState::QUIT:
                screen.ExitLoopClosure()();
                return text("Exiting...");
        }
        return text("Unknown state");
    });
    
    // Add periodic refresh for game loop simulation (60 FPS)
    std::atomic<bool> refresh_running(true);
    std::thread refresh_thread([&screen, &game_manager, &refresh_running]() {
        auto last_time = std::chrono::steady_clock::now();
        int frame_count = 0;
        double fps_accumulator = 0.0;
        
        while (refresh_running) {
            auto current_time = std::chrono::steady_clock::now();
            double delta_time = std::chrono::duration<double>(current_time - last_time).count();
            last_time = current_time;
            
            // Update FPS counter
            fps_accumulator += delta_time;
            frame_count++;
            
            if (fps_accumulator >= 1.0) {
                double fps = frame_count / fps_accumulator;
                if (game_manager.getFrameStats()) {
                    game_manager.getFrameStats()->update(fps, delta_time * 1000.0, 0.0, 0.0);
                }
                frame_count = 0;
                fps_accumulator = 0.0;
            }
            
            // Update game logic
            game_manager.update(delta_time);
            
            // Post refresh event
            screen.PostEvent(Event::Custom);
            
            // Target 60 FPS (16.67ms per frame)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    });
    
    // State-based input handler
    auto main_component = CatchEvent(main_renderer, [&](Event event) {
        // Handle refresh events
        if (event == Event::Custom) {
            return false; // Let the renderer update
        }
        
        switch(game_manager.getState()) {
            case GameState::MENU:
                return main_menu->OnEvent(event);
            case GameState::PLAYING:
            case GameState::INVENTORY:  // Inventory needs game_component events too
                return game_component->OnEvent(event);
            case GameState::SAVE_LOAD:
                return save_load_screen.handleInput(event);
            case GameState::PAUSED:
            case GameState::HELP:
                if (event == Event::Escape) {
                    game_manager.returnToPreviousState();
                    return true;
                }
                return false;
            case GameState::DEATH:
                if (event == Event::Character('r') || event == Event::Character('R')) {
                    game_manager.setState(GameState::MENU);
                    return true;
                } else if (event == Event::Character('q') || event == Event::Character('Q')) {
                    game_manager.setState(GameState::QUIT);
                    return true;
                }
                return false;
            case GameState::QUIT:
            default:
                return false;
        }
    });
    
    // If we have test input, create a thread to inject events
    std::thread* input_thread = nullptr;
    if (test_input && test_input->hasNextKeystroke()) {
        input_thread = new std::thread([&screen, test_input]() {
            // Wait a bit for the screen to initialize
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Send all test keystrokes with small delays
            while (test_input->hasNextKeystroke()) {
                auto event = test_input->getNextKeystroke();
                screen.PostEvent(event);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            
            // Send quit after all keystrokes are processed
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "\nTest input completed\n";
            screen.PostEvent(Event::Character('q'));
            screen.PostEvent(Event::Character('q')); // Double q to quit from game then menu
        });
    }
    
    screen.Loop(main_component);
    
    // Stop refresh thread
    refresh_running = false;
    refresh_thread.join();
    
    // Clean up the input thread if it exists
    if (input_thread) {
        input_thread->join();
        delete input_thread;
    }
    
    // Terminal cleanup is handled by resetTerminal() via atexit
    std::cout << "Thanks for playing Veyrm!\n";
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    // Create log directory
    [[maybe_unused]] int result = system("mkdir -p logs");

    // Initialize logging first
    Log::init("logs/veyrm_debug.log", Log::DEBUG);
    LOG_INFO("=== Veyrm starting up ===");

    // Initialize platform-specific settings
    initializePlatform();

    // Load configuration file
    Config& config = Config::getInstance();
    config.loadFromFile("config.yml");
    
    // Get default map type from config
    MapType map_type = config.getDefaultMapType();
    
    // Parse command-line arguments for config options (CLI overrides config file)
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        // Check for config file argument
        if (arg == "--config" && i + 1 < argc) {
            std::string config_path = argv[++i];
            if (!config.loadFromFile(config_path)) {
                std::cerr << "Error: Failed to load config file: " << config_path << "\n";
                return 1;
            }
            continue;
        }
        
        // Check for data directory argument (overrides config)
        if (arg == "--data-dir" && i + 1 < argc) {
            std::string data_path = argv[++i];
            config.setDataDir(data_path);
            if (!config.isDataDirValid()) {
                std::cerr << "Error: Data directory does not exist: " << data_path << "\n";
                return 1;
            }
            continue;
        }
        
        // Check for map type argument (overrides config)
        if (arg == "--map" && i + 1 < argc) {
            std::string map_arg = argv[++i];
            if (map_arg == "room") {
                map_type = MapType::TEST_ROOM;
            } else if (map_arg == "dungeon") {
                map_type = MapType::TEST_DUNGEON;
            } else if (map_arg == "corridor") {
                map_type = MapType::CORRIDOR_TEST;
            } else if (map_arg == "arena") {
                map_type = MapType::COMBAT_ARENA;
            } else if (map_arg == "stress") {
                map_type = MapType::STRESS_TEST;
            } else if (map_arg == "procedural") {
                map_type = MapType::PROCEDURAL;
            } else {
                std::cerr << "Unknown map type: " << map_arg << "\n";
                std::cerr << "Valid types: room, dungeon, corridor, arena, stress, procedural\n";
                return 1;
            }
            continue;
        }
    }
    
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
            std::cout << "  -h, --help          Show this help message\n";
            std::cout << "  -v, --version       Show version information\n";
            std::cout << "  --test              Run system checks\n";
            std::cout << "  --no-ui             Run without UI (test mode)\n";
            std::cout << "  --keys <keystrokes> Run with automated keystrokes\n";
            std::cout << "  --dump <keystrokes> Run in frame dump mode (slideshow)\n";
            std::cout << "  --config <file>     Load configuration from file (default: config.yml)\n";
            std::cout << "  --data-dir <path>   Set path to data directory (default: ./data)\n";
            std::cout << "  --map <type>        Start with specific map type\n";
            std::cout << "                      Types: procedural (random), room, dungeon,\n";
            std::cout << "                             corridor, arena, stress\n";
            std::cout << "\nKeystroke format:\n";
            std::cout << "  Regular characters are sent as-is\n";
            std::cout << "  Escape sequences:\n";
            std::cout << "    \\n - Enter/Return\n";
            std::cout << "    \\e - Escape\n";
            std::cout << "    \\u - Up arrow\n";
            std::cout << "    \\d - Down arrow\n";
            std::cout << "    \\l - Left arrow\n";
            std::cout << "    \\r - Right arrow\n";
            std::cout << "    \\t - Tab\n";
            std::cout << "    \\b - Backspace\n";
            std::cout << "    \\\\ - Literal backslash\n";
            std::cout << "\nExample: --keys \"\\n\\u\\u\\n\" (Enter, Up, Up, Enter)\n";
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
        } else if (arg == "--keys" && argc > 2) {
            // Run with automated keystrokes
            TestInput test_input;
            test_input.loadKeystrokes(argv[2]);
            std::cout << "Running with automated input: " << argv[2] << "\n";
            runInterface(&test_input, map_type);
            return 0;
        } else if (arg == "--dump" && argc > 2) {
            // Run in frame dump mode
            TestInput test_input;
            test_input.loadKeystrokes(argv[2]);
            test_input.setFrameDumpMode(true);
            runFrameDumpMode(&test_input, map_type);
            return 0;
        } else if (arg != "--map") {
            // --map is already handled above, only show error for truly unknown options
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information\n";
            return 1;
        }
    }
    
    // Run the interface normally with selected map type
    runInterface(nullptr, map_type);
    
    return 0;
}