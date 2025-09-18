#include "ui/game_view.h"
#include "game_state.h"
#include "renderer.h"
#include "status_bar.h"
#include "inventory_renderer.h"
#include "message_log.h"
#include "layout_system.h"
#include "log.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace ui {

GameView::GameView(GameManager* gm, ftxui::ScreenInteractive* screen)
    : game_manager(gm)
    , screen_ref(screen) {

    // Initialize renderers
    map_renderer = std::make_unique<MapRenderer>();
    status_bar = std::make_unique<StatusBar>();
    inventory_renderer = std::make_unique<InventoryRenderer>(gm);
}

GameView::~GameView() {
    // Destructor defined here for unique_ptr with forward declarations
}

ftxui::Component GameView::createComponent() {
    // Create the main game layout
    auto layout = Renderer([this] {
        switch (current_mode) {
            case Mode::INVENTORY:
                return renderInventoryMode();
            case Mode::HELP:
                return renderHelpMode();
            case Mode::DIALOG:
                return renderDialogMode();
            case Mode::NORMAL:
            default:
                return renderNormalMode();
        }
    });

    // Add input handling
    layout = CatchEvent(layout, [this](Event event) {
        if (controller_callbacks.onInput) {
            return controller_callbacks.onInput(event);
        }
        return false;
    });

    return layout;
}

ftxui::Component GameView::createMapPanel() {
    return Renderer([this] {
        if (!map_renderer) {
            return text("Map renderer not initialized") | border;
        }

        // Get the rendered map
        auto* map = game_manager->getMap();
        if (!map) {
            return text("No map available") | border;
        }
        auto map_element = map_renderer->render(*map, *game_manager);

        // Add border and title
        return vbox({
            text("Map") | bold,
            separator(),
            map_element
        }) | border;
    });
}

ftxui::Component GameView::createStatusPanel() {
    return Renderer([this] {
        if (!status_bar) {
            return text("Status bar not initialized") | border;
        }

        // Get status elements
        auto status_element = status_bar->render(*game_manager);

        return vbox({
            text("Status") | bold,
            separator(),
            status_element
        }) | border;
    });
}

ftxui::Component GameView::createMessagePanel() {
    return Renderer([this] {
        Elements messages;

        // Get messages from game manager's message log
        if (game_manager && game_manager->getMessageLog()) {
            auto log_messages = game_manager->getMessageLog()->getMessages();

            // Show last 5 messages
            int start = std::max(0, static_cast<int>(log_messages.size()) - 5);
            for (size_t i = start; i < log_messages.size(); i++) {
                messages.push_back(text(log_messages[i]));
            }
        }

        // Add any prompt if active
        if (show_prompt && !current_prompt.empty()) {
            messages.push_back(separator());
            messages.push_back(text(current_prompt) | color(Color::Yellow));
        }

        return vbox({
            text("Messages") | bold,
            separator(),
            vbox(messages)
        }) | border;
    });
}

ftxui::Component GameView::createInventoryOverlay() {
    return Renderer([this] {
        if (!inventory_renderer) {
            return text("Inventory not available") | center | border;
        }

        return inventory_renderer->render() | border;
    });
}

ftxui::Component GameView::createHelpOverlay() {
    return Renderer([] {
        Elements help_lines;

        help_lines.push_back(text("HELP") | bold | center);
        help_lines.push_back(separator());
        help_lines.push_back(text(""));
        help_lines.push_back(text("Movement:"));
        help_lines.push_back(text("  Arrow keys or hjkl - Move"));
        help_lines.push_back(text("  7 9 - Diagonal movement"));
        help_lines.push_back(text("  1 3 - Diagonal movement"));
        help_lines.push_back(text("  . or 5 - Wait"));
        help_lines.push_back(text(""));
        help_lines.push_back(text("Actions:"));
        help_lines.push_back(text("  o - Open/close doors"));
        help_lines.push_back(text("  g - Get/pick up items"));
        help_lines.push_back(text("  i - Inventory"));
        help_lines.push_back(text("  > - Go down stairs"));
        help_lines.push_back(text("  < - Go up stairs"));
        help_lines.push_back(text(""));
        help_lines.push_back(text("Game:"));
        help_lines.push_back(text("  S - Save game"));
        help_lines.push_back(text("  L - Load game"));
        help_lines.push_back(text("  q - Quit to menu"));
        help_lines.push_back(text("  ? - This help"));
        help_lines.push_back(text(""));
        help_lines.push_back(separator());
        help_lines.push_back(text("Press ESC to close") | center);

        return vbox(help_lines) | border | center;
    });
}

ftxui::Component GameView::createPromptOverlay() {
    return Renderer([this] {
        if (current_prompt.empty()) {
            return text("");
        }

        return vbox({
            text(current_prompt) | center,
            separator(),
            text("Press direction key or ESC to cancel") | center
        }) | border | center;
    });
}

ftxui::Element GameView::renderNormalMode() {
    // Create three-panel layout: map, status, messages
    // Get map and status elements
    ftxui::Element map_element = text("Map");
    ftxui::Element status_element = text("Status");

    if (map_renderer && game_manager && game_manager->getMap()) {
        map_element = map_renderer->render(*game_manager->getMap(), *game_manager);
    }

    if (status_bar && game_manager) {
        status_element = status_bar->render(*game_manager);
    }

    // Get messages
    Elements messages;
    if (game_manager && game_manager->getMessageLog()) {
        auto log_messages = game_manager->getMessageLog()->getMessages();
        size_t start = log_messages.size() > 5 ? log_messages.size() - 5 : 0;
        for (size_t i = start; i < log_messages.size(); i++) {
            messages.push_back(text(log_messages[i]));
        }
    }

    // Build layout
    return hbox({
        vbox({
            map_element | flex | border,
            vbox(messages) | border | size(HEIGHT, EQUAL, 7)
        }) | flex,
        status_element | border | size(WIDTH, EQUAL, 30)
    });
}

ftxui::Element GameView::renderInventoryMode() {
    // Overlay inventory on top of normal game view
    auto game_view = renderNormalMode();
    auto inventory_overlay = inventory_renderer ?
        inventory_renderer->render() | border | center :
        text("Inventory") | border | center;

    return dbox({
        game_view,
        inventory_overlay
    });
}

ftxui::Element GameView::renderHelpMode() {
    // Overlay help on top of normal game view
    auto game_view = renderNormalMode();
    auto help_overlay = createHelpOverlay()->Render();

    return dbox({
        game_view,
        help_overlay
    });
}

ftxui::Element GameView::renderDialogMode() {
    // Overlay prompt/dialog on top of normal game view
    auto game_view = renderNormalMode();

    if (show_prompt && !current_prompt.empty()) {
        auto prompt_overlay = vbox({
            text(current_prompt) | center,
            separator(),
            text("Press direction key or ESC to cancel") | center
        }) | border | center;

        return dbox({
            game_view,
            prompt_overlay
        });
    }

    return game_view;
}

void GameView::showMessage(const std::string& message) {
    // Add to buffer for display
    message_buffer.push_back(message);

    // Keep buffer size reasonable
    if (message_buffer.size() > 100) {
        message_buffer.erase(message_buffer.begin());
    }

    refresh();
}

void GameView::showPrompt(const std::string& prompt) {
    current_prompt = prompt;
    show_prompt = true;
    setMode(Mode::DIALOG);
    refresh();
}

void GameView::clearPrompt() {
    current_prompt.clear();
    show_prompt = false;
    setMode(Mode::NORMAL);
    refresh();
}

void GameView::refresh() {
    if (controller_callbacks.onRefresh) {
        controller_callbacks.onRefresh();
    }
}

void GameView::showInventory(bool show) {
    setMode(show ? Mode::INVENTORY : Mode::NORMAL);
    refresh();
}

void GameView::updateStatus() {
    if (status_bar) {
        // Status bar updates itself from game state
        refresh();
    }
}

void GameView::updateMap() {
    if (map_renderer) {
        // Map renderer updates itself from game state
        refresh();
    }
}

} // namespace ui