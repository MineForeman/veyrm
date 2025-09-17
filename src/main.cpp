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
#include "login_screen.h"
#include "test_input.h"
#include "game_loop.h"
#include "ecs/game_world.h"
#include "ecs/health_component.h"
#include "ecs/stats_component.h"
#include "ecs/player_component.h"
#include "frame_stats.h"
#include "map_generator.h"
#include "config.h"

// Database and authentication
#include "db/database_manager.h"
#include "db/player_repository.h"
#include "auth/authentication_service.h"

// MVC components
#include "controllers/main_menu_controller.h"
#include "ui/main_menu_view.h"

// Platform-specific includes for UTF-8 support
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#endif

#include <filesystem>

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
    // Don't use _O_U8TEXT as it causes issues with FTXUI
    // _setmode(_fileno(stdout), _O_U8TEXT);

    // Enable virtual terminal processing for better terminal support
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE) {
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#else
    // Set locale for proper Unicode handling on Unix systems
    std::locale::global(std::locale(""));
#endif
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
    if constexpr (__cplusplus >= 202302L) {
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

// Menu state - needs to be accessible from input handler
static int menu_selected = 0;

/**
 * Create main menu component using MVC pattern
 */
ftxui::Component createMainMenu(GameManager* game_manager, [[maybe_unused]] ftxui::ScreenInteractive* screen,
                               auth::AuthenticationService* auth_service,
                               LoginScreen* login_screen,
                               int* user_id, std::string* session_token, std::string* username) {
    using namespace ftxui;

    // Create MVC components
    static std::unique_ptr<controllers::MainMenuController> controller;
    static std::unique_ptr<ui::MainMenuView> view;

    if (!controller) {
        controller = std::make_unique<controllers::MainMenuController>(
            game_manager,
            auth_service,
            login_screen
        );
    }

    if (!view) {
        view = std::make_unique<ui::MainMenuView>();
    }

    // Update authentication state in controller when user_id changes
    if (user_id && session_token && username) {
        // These need to be set via proper methods or made public
        // For now, we'll work with the controller's public interface
    }

    // Set controller callbacks
    controllers::MainMenuController::ViewCallbacks callbacks;
    callbacks.showMessage = [](const std::string& msg) {
        LOG_INFO("Menu message: " + msg);
    };
    callbacks.showError = [](const std::string& error) {
        LOG_ERROR("Menu error: " + error);
    };
    callbacks.refreshMenu = []() {
        // This will be handled by view refresh
    };
    callbacks.exitApplication = []() {
        // This will be handled by game_manager state
    };
    controller->setViewCallbacks(callbacks);

    // Create view callbacks for controller
    ui::MainMenuView::ControllerCallbacks view_callbacks;
    view_callbacks.onMenuSelect = [controller = controller.get()](int index) {
        if (controller->isAuthenticated()) {
            // Handle authenticated menu selections
            switch(index) {
                case 0: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::NEW_GAME); break;
                case 1: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::CONTINUE); break;
                case 2: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::CLOUD_SAVES); break;
                case 3: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::LEADERBOARDS); break;
                case 4: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::SETTINGS); break;
                case 5: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::PROFILE); break;
                case 6: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::LOGOUT); break;
                case 7: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::ABOUT); break;
                case 8: controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::QUIT); break;
            }
        } else {
            // Handle unauthenticated menu selections
            switch(index) {
                case 0: controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::LOGIN); break;
                case 1: controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::REGISTER); break;
                case 2: controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::ABOUT); break;
                case 3: controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::QUIT); break;
            }
        }
    };
    view_callbacks.onAboutToggle = [controller = controller.get()]() {
        controller->toggleAbout();
    };
    view_callbacks.onExit = []() {
        // Handled by game manager
    };
    view_callbacks.isAuthenticated = [controller = controller.get()]() {
        return controller->isAuthenticated();
    };
    view_callbacks.getUsername = [controller = controller.get()]() {
        return controller->getUsername();
    };
    view_callbacks.getAuthStatus = [controller = controller.get()]() {
        return controller->getAuthStatus();
    };

    view->setControllerCallbacks(view_callbacks);
    view->setAuthenticated(controller->isAuthenticated());

    // Create the FTXUI component that integrates with the MVC pattern
    using namespace ftxui;

    // Menu state
    static int selected = 0;
    static std::vector<std::string> menu_entries;

    // Update menu entries based on authentication state
    menu_entries.clear();

    if (controller->isAuthenticated()) {
        menu_entries = {
            "New Game",
            "Continue",
            "Cloud Saves",
            "Leaderboards",
            "Settings",
            "Profile",
            "Logout",
            "About",
            "Quit"
        };
    } else {
        // If not authenticated, only show login options
        menu_entries = {
            "Login",
            "Register",
            "About",
            "Quit"
        };
    }

    auto menu = Menu(&menu_entries, &selected);

    // Create the main component with renderer
    auto component = Renderer(menu, [controller = controller.get(), menu, user_id, session_token, username] {
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
        if (selected == 3 || (controller->isAuthenticated() && selected == 7) || (!controller->isAuthenticated() && selected == 2)) {  // Show about when About is selected
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
        
        // Clean status line with auth status
        // Re-check authentication state since it might have changed
        bool is_currently_authenticated = (user_id && *user_id > 0);
        std::string auth_status;
        if (is_currently_authenticated && user_id && session_token) {
            if (username && !username->empty()) {
                auth_status = " | Logged in as: " + *username;
            } else {
                auth_status = " | Logged in (ID: " + std::to_string(*user_id) + ")";
            }
        } else {
            auth_status = " | Playing as Guest";
        }

        auto status = hbox({
            text("[↑↓] Navigate  [Enter] Select  [Q] Quit" + auth_status) | dim,
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
    // Capture pointers explicitly
    component |= CatchEvent([controller = controller.get(), selected = &selected, user_id, session_token, username](Event event) {
        // Handle Enter key for menu selection
        if (event == Event::Return) {
            if (controller->isAuthenticated()) {
                // Authenticated menu
                switch(*selected) {
                    case 0: // New Game
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::NEW_GAME);
                        break;
                    case 1: // Continue
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::CONTINUE);
                        break;
                    case 2: // Cloud Saves
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::CLOUD_SAVES);
                        break;
                    case 3: // Leaderboards
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::LEADERBOARDS);
                        break;
                    case 4: // Settings
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::SETTINGS);
                        break;
                    case 5: // Profile
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::PROFILE);
                        break;
                    case 6: // Logout
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::LOGOUT);
                        if (user_id) *user_id = 0;
                        if (session_token) session_token->clear();
                        if (username) username->clear();
                        break;
                    case 7: // About
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::ABOUT);
                        break;
                    case 8: // Quit
                        controller->handleAuthenticatedSelection(controllers::MainMenuController::AuthenticatedOption::QUIT);
                        break;
                }
            } else {
                // Unauthenticated menu - only login/register/about/quit
                switch(*selected) {
                    case 0: // Login
                        controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::LOGIN);
                        // Update auth state if successful
                        if (controller->isAuthenticated()) {
                            if (user_id) *user_id = controller->getUserId();
                            // Session token is managed internally by controller
                            if (username) *username = controller->getUsername();
                        }
                        break;
                    case 1: // Register
                        controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::REGISTER);
                        // Update auth state if successful
                        if (controller->isAuthenticated()) {
                            if (user_id) *user_id = controller->getUserId();
                            // Session token is managed internally by controller
                            if (username) *username = controller->getUsername();
                        }
                        break;
                    case 2: // About
                        controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::ABOUT);
                        break;
                    case 3: // Quit
                        controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::QUIT);
                        break;
                }
            }
            return true;
        }
        if (event == Event::Character('q') || event == Event::Escape) {
            controller->handleUnauthenticatedSelection(controllers::MainMenuController::UnauthenticatedOption::QUIT);
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
    // Dummy auth state for dump mode
    int dump_user_id = 0;
    std::string dump_session_token;
    std::string dump_username;
    Component main_menu = createMainMenu(&game_manager, &screen,
                                        nullptr,  // No auth service in dump mode
                                        nullptr,  // No login screen in dump mode
                                        &dump_user_id, &dump_session_token, &dump_username);
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
                case GameState::LOGIN:
                    return vbox({
                        text("LOGIN SCREEN") | bold | center,
                        separator(),
                        text("Authentication not available in dump mode") | center,
                        text("Press ESC to return") | center
                    }) | border;
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
            case GameState::LOGIN: std::cout << "LOGIN"; break;
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
            case GameState::LOGIN:
                // In dump mode, just escape back to menu
                if (event == Event::Escape) {
                    game_manager.setState(GameState::MENU);
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

    // Authentication state
    int user_id = 0;
    std::string session_token;
    std::string username;  // Store the logged-in username
    std::string player_name = "Hero";

    // Initialize authentication service (always required)
    std::unique_ptr<db::PlayerRepository> player_repo;
    std::unique_ptr<auth::AuthenticationService> auth_service;
    std::unique_ptr<LoginScreen> login_screen;
    Component login_component;

    if (!db::DatabaseManager::getInstance().isInitialized()) {
        LOG_ERROR("Database not initialized - cannot continue");
        std::cerr << "Error: Database connection required. Please ensure PostgreSQL is running.\n";
        return;
    }

    player_repo = std::make_unique<db::PlayerRepository>(db::DatabaseManager::getInstance());
    auth_service = std::make_unique<auth::AuthenticationService>(*player_repo, db::DatabaseManager::getInstance());
    login_screen = std::make_unique<LoginScreen>(*auth_service);

    // Set callback for successful login
    login_screen->setOnLoginSuccess([&user_id, &session_token, &game_manager](int uid, const std::string& token) {
        user_id = uid;
        session_token = token;
        // TODO: Get player name from database
        game_manager.setState(GameState::MENU);
    });

    // Check if user needs to authenticate first
    if (user_id == 0 && !test_input) {
        // Not authenticated and not in test mode - go directly to login
        LOG_INFO("No authenticated user - launching login screen");
        auto result = login_screen->run();
        if (result == LoginScreen::Result::SUCCESS) {
            user_id = login_screen->getUserId();
            session_token = login_screen->getSessionToken();
            // Get username from database
            try {
                username = db::DatabaseManager::getInstance().executeQuery([&user_id](db::Connection& conn) {
                    auto result = conn.execParams(
                        "SELECT username FROM users WHERE id = $1",
                        {std::to_string(user_id)}
                    );
                    if (result.isOk() && result.numRows() > 0) {
                        return result.getValue(0, 0);
                    }
                    return std::string("");
                });
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to get username: " + std::string(e.what()));
            }
            LOG_INFO("User authenticated at startup: " + username + " (ID=" + std::to_string(user_id) + ")");
        } else {
            LOG_INFO("Login cancelled or failed - exiting");
            return;  // Exit if user cancels login
        }
    }

    // Create components
    Component main_menu = createMainMenu(&game_manager, &screen,
                                        auth_service.get(),
                                        login_screen.get(),
                                        &user_id, &session_token, &username);
    GameScreen game_screen(&game_manager, &screen);
    Component game_component = game_screen.Create();
    SaveLoadScreen save_load_screen(&game_manager);
    Component save_load_component = save_load_screen.create();

    // State-based renderer
    auto main_renderer = Renderer([&] {
        switch(game_manager.getState()) {
            case GameState::MENU:
                return main_menu->Render();
            case GameState::LOGIN:
                // LoginScreen will be handled separately
                // Show a transitional message
                return vbox({
                    text("Launching authentication screen...") | center,
                    separator(),
                    text("Please wait...") | center
                }) | border | center;
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
                    text("VEYRM HELP") | bold | center,
                    separator(),
                    text("MOVEMENT:") | bold | color(Color::Yellow),
                    text("  Arrow keys    Move in cardinal directions"),
                    text("  Numpad 1-9    Move (including diagonals)"),
                    text("  .             Wait a turn"),
                    text(""),
                    text("ACTIONS:") | bold | color(Color::Yellow),
                    text("  g             Get/pickup item"),
                    text("  o             Open/close door"),
                    text("  i             Open inventory"),
                    text("  u             Use item (in inventory)"),
                    text("  D             Drop item (uppercase D)"),
                    text("  E             Examine item (uppercase E)"),
                    text(""),
                    text("INTERFACE:") | bold | color(Color::Yellow),
                    text("  ?             Show this help"),
                    text("  S             Save game (uppercase S)"),
                    text("  L             Load game (uppercase L)"),
                    text("  q/Q           Quit to main menu"),
                    text("  ESC           Cancel/go back"),
                    text("  Enter         Confirm selection"),
                    text(""),
                    text("COMBAT:") | bold | color(Color::Yellow),
                    text("  Bump to attack - move into an enemy to attack"),
                    text(""),
                    text("DEBUG:") | bold | color(Color::Yellow),
                    text("  F1            Toggle debug mode"),
                    separator(),
                    text("Press ESC to return to game") | dim
                }) | border | size(WIDTH, EQUAL, 60);
            case GameState::DEATH: {
                // Get player stats from ECS
                std::string player_stats = "Lvl 1 | Turn " + std::to_string(game_manager.getDeathTurn());
                std::string hp_info = "Final HP: 0";
                std::string cause_info = "Cause: " + game_manager.getDeathCause();

                // Get additional stats if ECS world is available
                if (auto* ecs_world = game_manager.getECSWorld()) {
                    if (auto* player = ecs_world->getPlayerEntity()) {
                        if (auto* health = player->getComponent<ecs::HealthComponent>()) {
                            hp_info = "Final HP: " + std::to_string(health->hp) + "/" + std::to_string(health->max_hp);
                        }
                        if (auto* stats = player->getComponent<ecs::StatsComponent>()) {
                            // Use a calculated level or just show stats
                            int player_level = 1; // Default level
                            player_stats = "Lvl " + std::to_string(player_level) + " | STR:" + std::to_string(stats->strength) + " | Turn " + std::to_string(game_manager.getDeathTurn());
                        }
                    }
                }

                return vbox({
                    text("") | size(WIDTH, EQUAL, 1),
                    hbox({
                        text("         "),
                        vbox({
                            text("╔═══════════════════════════════════════════════════╗") | color(Color::Red),
                            text("║                                                   ║") | color(Color::Red),
                            text("║                Y O U   D I E D                    ║") | color(Color::Red) | bold,
                            text("║                                                   ║") | color(Color::Red),
                            text("║     Your adventure has come to an end...         ║") | color(Color::White),
                            text("║                                                   ║") | color(Color::Red),
                            text("╠═══════════════════════════════════════════════════╣") | color(Color::Red),
                            text("║  " + player_stats + std::string(49 - player_stats.length(), ' ') + "║") | color(Color::Yellow),
                            text("║  " + hp_info + std::string(49 - hp_info.length(), ' ') + "║") | color(Color::Cyan),
                            text("║  " + cause_info + std::string(49 - cause_info.length(), ' ') + "║") | color(Color::Magenta),
                            text("║                                                   ║") | color(Color::Red),
                            text("║  The darkness claims another soul in the depths  ║") | color(Color::White),
                            text("║  of Veyrmspire. Your bones join countless        ║") | color(Color::White),
                            text("║  others in the Spiral Vaults...                  ║") | color(Color::White),
                            text("║                                                   ║") | color(Color::Red),
                            text("╠═══════════════════════════════════════════════════╣") | color(Color::Red),
                            text("║                                                   ║") | color(Color::Red),
                            text("║      [R] Return to Main Menu                      ║") | color(Color::Yellow),
                            text("║      [Q] Quit Game                                ║") | color(Color::Yellow),
                            text("║                                                   ║") | color(Color::Red),
                            text("╚═══════════════════════════════════════════════════╝") | color(Color::Red)
                        }),
                        text("         ")
                    }) | center,
                    text("") | size(WIDTH, EQUAL, 1)
                }) | center | size(HEIGHT, EQUAL, 25);
            }
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
            case GameState::LOGIN:
                // Launch login screen when we first enter LOGIN state
                {
                    static bool login_launched = false;
                    if (!login_launched && login_screen) {
                        login_launched = true;

                        // Determine mode based on which menu item was selected
                        bool is_register = (menu_selected == 1);  // Register is 2nd item (index 1) in unauthenticated menu
                        login_screen->setMode(is_register ? LoginScreen::Mode::REGISTER : LoginScreen::Mode::LOGIN);

                        // Stop the refresh thread
                        refresh_running = false;
                        refresh_thread.join();

                        // Exit the current screen
                        screen.ExitLoopClosure()();

                        // Run the login screen
                        LOG_INFO("Running LoginScreen...");
                        auto result = login_screen->run();

                        if (result == LoginScreen::Result::SUCCESS) {
                            user_id = login_screen->getUserId();
                            session_token = login_screen->getSessionToken();
                            LOG_INFO("Authentication successful: ID=" + std::to_string(user_id));
                        }

                        // Reset state
                        login_launched = false;
                        game_manager.setState(GameState::MENU);

                        // Restart everything
                        return true;
                    }

                    if (event == Event::Escape) {
                        login_launched = false;
                        game_manager.setState(GameState::MENU);
                        return true;
                    }
                }
                return false;
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
    // Create log directory using std::filesystem (cross-platform)
    std::filesystem::create_directories("logs");

    // Initialize logging first
    Log::init("logs/veyrm_debug.log", Log::DEBUG);
    LOG_INFO("=== Veyrm starting up ===");

    // Initialize platform-specific settings
    initializePlatform();

    // Load configuration file
    Config& config = Config::getInstance();
    config.loadFromFile("config.yml");

    // Initialize database (REQUIRED)
    {
        LOG_INFO("Initializing database connection...");
        db::DatabaseConfig db_config;

        // Try environment variables first
        const char* db_host = std::getenv("DB_HOST");
        const char* db_port = std::getenv("DB_PORT");
        const char* db_name = std::getenv("DB_NAME");
        const char* db_user = std::getenv("DB_USER");
        const char* db_pass = std::getenv("DB_PASS");  // Changed from DB_PASSWORD to match .env

        // Use environment variables or defaults
        db_config.host = db_host ? db_host : "localhost";
        db_config.port = db_port ? std::stoi(db_port) : 5432;
        db_config.database = db_name ? db_name : "veyrm_db";
        db_config.username = db_user ? db_user : "veyrm_admin";
        db_config.password = db_pass ? db_pass : "changeme_to_secure_password";

        LOG_INFO("Attempting database connection with:");
        LOG_INFO("  Host: " + db_config.host);
        LOG_INFO("  Port: " + std::to_string(db_config.port));
        LOG_INFO("  Database: " + db_config.database);
        LOG_INFO("  Username: " + db_config.username);
        LOG_INFO("  Password: " + std::string(db_config.password.empty() ? "NOT SET" : "SET"));

        try {
            db::DatabaseManager::getInstance().initialize(db_config);
            LOG_INFO("Database connection established successfully");

            // Create tables if they don't exist
            if (db::DatabaseManager::getInstance().createTables()) {
                LOG_INFO("Database tables verified/created");
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Database initialization failed: " + std::string(e.what()));
            std::cerr << "Error: Database connection required. Please ensure PostgreSQL is running.\n";
            std::cerr << "Error details: " << e.what() << "\n";
            return 1;  // Exit if database is not available
        }
    }
    
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