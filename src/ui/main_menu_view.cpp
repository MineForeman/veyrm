#include "ui/main_menu_view.h"
#include "ui/components/button.h"
#include "ui/components/panel.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

namespace ui {

MainMenuView::MainMenuView()
    : selected_index(0)
    , is_authenticated(false)
    , show_about(false)
    , result(Result::NONE)
    , show_status(false)
    , show_error(false)
    , screen(ScreenInteractive::Fullscreen()) {

    updateMenuEntries();
}

MainMenuView::~MainMenuView() = default;

void MainMenuView::setControllerCallbacks(const ControllerCallbacks& callbacks) {
    controller_callbacks = callbacks;
}

MainMenuView::Result MainMenuView::run() {
    // Reset state
    result = Result::NONE;
    clearMessages();

    // Check authentication state
    if (controller_callbacks.isAuthenticated) {
        is_authenticated = controller_callbacks.isAuthenticated();
    }

    updateMenuEntries();

    // Create main component
    main_component = createMainComponent();

    // Run the screen
    screen.Loop(main_component);

    return result;
}

void MainMenuView::setAuthenticated(bool authenticated) {
    is_authenticated = authenticated;
    updateMenuEntries();
}

void MainMenuView::showMessage(const std::string& message) {
    status_message = message;
    show_status = true;
    show_error = false;
}

void MainMenuView::showError(const std::string& error) {
    error_message = error;
    show_error = true;
    show_status = false;
}

void MainMenuView::refresh() {
    // Update authentication state
    if (controller_callbacks.isAuthenticated) {
        is_authenticated = controller_callbacks.isAuthenticated();
    }
    updateMenuEntries();

    // Recreate the main component with updated menu entries
    main_component = createMainComponent();

    // Force screen refresh
    screen.PostEvent(Event::Custom);
}

void MainMenuView::exit() {
    result = Result::CANCELLED;
    screen.ExitLoopClosure()();
}

Component MainMenuView::createMainComponent() {
    auto menu = Menu(&menu_entries, &selected_index);

    return Renderer(menu, [this, menu] {
        Elements elements;

        // Title
        elements.push_back(createTitleBanner());

        // Main content area
        Elements content;

        // Menu
        auto menu_element = menu->Render() | center;
        content.push_back(menu_element);

        // About box (if visible)
        if (show_about) {
            content.push_back(text(" "));
            content.push_back(createAboutBox());
        }

        elements.push_back(flex(vbox(content)));

        // Status bar
        elements.push_back(separator());

        std::string auth_status = "";
        if (controller_callbacks.getAuthStatus) {
            auth_status = controller_callbacks.getAuthStatus();
        }

        elements.push_back(
            text("[↑↓] Navigate  [Enter] Select  [Q] Quit" + auth_status) | dim | center
        );

        // Messages
        if (show_status && !status_message.empty()) {
            elements.push_back(text(status_message) | color(Color::Green) | center);
        }

        if (show_error && !error_message.empty()) {
            elements.push_back(text(error_message) | color(Color::Red) | center);
        }

        return vbox(elements);
    }) | CatchEvent([this](Event event) {
        if (event == Event::Return) {
            handleSelection();
            return true;
        }

        if (event == Event::Character('q') || event == Event::Character('Q')) {
            if (controller_callbacks.onExit) {
                controller_callbacks.onExit();
            }
            exit();
            return true;
        }

        if (event == Event::Escape) {
            exit();
            return true;
        }

        return false;
    });
}

Element MainMenuView::createTitleBanner() const {
    return vbox({
        text(""),
        text("██╗   ██╗███████╗██╗   ██╗██████╗ ███╗   ███╗") | color(Color::Red) | center,
        text("██║   ██║██╔════╝╚██╗ ██╔╝██╔══██╗████╗ ████║") | color(Color::RedLight) | center,
        text("██║   ██║█████╗   ╚████╔╝ ██████╔╝██╔████╔██║") | color(Color::Yellow) | center,
        text("╚██╗ ██╔╝██╔══╝    ╚██╔╝  ██╔══██╗██║╚██╔╝██║") | color(Color::YellowLight) | center,
        text(" ╚████╔╝ ███████╗   ██║   ██║  ██║██║ ╚═╝ ██║") | color(Color::Green) | center,
        text("  ╚═══╝  ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝     ╚═╝") | color(Color::GreenLight) | center,
        text(""),
        text("A Modern Roguelike Experience") | dim | center,
        text("")
    });
}

Element MainMenuView::createAboutBox() const {
    return vbox({
        separator(),
        text("About Veyrm") | bold | center,
        separator(),
        text("Version 0.1.0") | center,
        text("") | center,
        text("A modern roguelike dungeon crawler") | center,
        text("featuring classic gameplay with") | center,
        text("contemporary design and features.") | center,
        text("") | center,
        text("Built with C++23 and FTXUI") | dim | center,
        text("© 2025 Veyrm Development Team") | dim | center,
        separator()
    }) | border | size(WIDTH, EQUAL, 45);
}

void MainMenuView::updateMenuEntries() {
    menu_entries.clear();

    // Main menu is only shown after authentication, so always show authenticated options
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

    // Ensure selected index is valid
    if (selected_index >= static_cast<int>(menu_entries.size())) {
        selected_index = 0;
    }
}

void MainMenuView::handleSelection() {
    if (controller_callbacks.onMenuSelect) {
        controller_callbacks.onMenuSelect(selected_index);
    }

    // Check if we should toggle about
    if ((is_authenticated && selected_index == 7) ||  // About in authenticated menu
        (!is_authenticated && selected_index == 2)) {  // About in unauthenticated menu
        if (controller_callbacks.onAboutToggle) {
            controller_callbacks.onAboutToggle();
            show_about = !show_about;
        }
    }

    result = Result::SELECTION_MADE;
}

void MainMenuView::clearMessages() {
    status_message.clear();
    error_message.clear();
    show_status = false;
    show_error = false;
}

} // namespace ui