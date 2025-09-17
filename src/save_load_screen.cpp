#include "save_load_screen.h"
#include "controllers/save_load_controller.h"
#include "services/save_game_service.h"
#include "ui/save_load_view.h"
#include "game_state.h"
#include "game_serializer.h"
#include "message_log.h"
#include "log.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>
#include <iomanip>

using namespace ftxui;

SaveLoadScreen::SaveLoadScreen(GameManager* gm)
    : game_manager(gm), selected_slot(0), save_mode(true) {

    // Initialize MVC components
    save_service = std::make_shared<services::SaveGameService>(
        gm->getSerializer()
    );

    controller = std::make_unique<controllers::SaveLoadController>(
        gm,
        save_service,
        nullptr  // Cloud service optional
    );

    view = std::make_unique<ui::SaveLoadView>();

    setupMVCCallbacks();
    refreshSaveList();
}

SaveLoadScreen::~SaveLoadScreen() {
    // Destructor defined here to allow unique_ptr with forward declaration
}

void SaveLoadScreen::reset() {
    selected_slot = 0;

    // Get the mode from GameManager
    save_mode = game_manager->getSaveMenuMode();

    // Update MVC components
    controller->setMode(save_mode ?
        controllers::SaveLoadController::Mode::SAVE :
        controllers::SaveLoadController::Mode::LOAD);

    view->setMode(save_mode ?
        ui::SaveLoadView::Mode::SAVE :
        ui::SaveLoadView::Mode::LOAD);

    refreshSaveList();
}

void SaveLoadScreen::refreshSaveList() {
    // Refresh through controller
    controller->refreshSlots();

    // Get updated list for backward compatibility
    auto slots = controller->getSaveSlots();
    save_infos.clear();

    for (const auto& slot : slots.slots) {
        SaveInfo info;
        info.exists = slot.exists;
        info.player_name = slot.player_name;
        info.depth = slot.depth;
        info.player_hp = slot.player_hp;
        info.player_max_hp = slot.player_max_hp;
        info.timestamp = slot.timestamp;
        info.filename = slot.filename;
        save_infos.push_back(info);
    }
}

std::vector<std::string> SaveLoadScreen::getSaveSlotDescriptions() const {
    std::vector<std::string> descriptions;

    for (int i = 0; i < MAX_SLOTS; i++) {
        descriptions.push_back(formatSaveInfo(i + 1));
    }

    return descriptions;
}

std::string SaveLoadScreen::formatSaveInfo(int slot) const {
    if (slot < 1 || slot > MAX_SLOTS) return "Invalid slot";

    const SaveInfo& info = save_infos[slot - 1];
    std::stringstream ss;
    ss << "Slot " << slot << ": ";

    if (info.exists) {
        ss << "Level " << info.depth;
        ss << " - " << info.player_name;
        ss << " (HP: " << info.player_hp << "/" << info.player_max_hp << ")";
        ss << " - " << info.timestamp;
    } else {
        ss << "Empty";
    }

    return ss.str();
}

void SaveLoadScreen::saveToSlot(int slot) {
    controller->handleSave(slot);
}

void SaveLoadScreen::loadFromSlot(int slot) {
    controller->handleLoad(slot);
}

void SaveLoadScreen::deleteSlot(int slot) {
    controller->handleDelete(slot);
}

Component SaveLoadScreen::create() {
    return Renderer([this] {
        // Always check mode from GameManager when rendering
        save_mode = game_manager->getSaveMenuMode();
        Elements elements;

        // Title
        std::string title = save_mode ? "SAVE GAME" : "LOAD GAME";
        elements.push_back(hbox({
            text(title) | bold | center
        }));
        elements.push_back(separator());

        // Instructions
        elements.push_back(text(save_mode ?
            "Select a slot to save your game:" :
            "Select a slot to load from:"));
        elements.push_back(text(" "));

        // Save slots
        auto descriptions = getSaveSlotDescriptions();
        for (int i = 0; i < MAX_SLOTS; i++) {
            bool is_selected = (i == selected_slot);
            auto slot_text = text(descriptions[i]);

            if (is_selected) {
                slot_text = slot_text | inverted;
            }

            // Show if slot has a save
            if (save_infos[i].exists) {
                if (!save_mode) {
                    // In load mode, highlight existing saves
                    slot_text = slot_text | color(Color::Green);
                } else {
                    // In save mode, warn about overwriting
                    slot_text = slot_text | color(Color::Yellow);
                }
            }

            elements.push_back(hbox({
                text(is_selected ? "> " : "  "),
                slot_text
            }));
        }

        elements.push_back(text(" "));
        elements.push_back(separator());

        // Controls
        elements.push_back(text("Controls:"));
        elements.push_back(text("  ↑/↓ or j/k - Navigate slots"));
        elements.push_back(text("  Enter - " + std::string(save_mode ? "Save" : "Load") + " selected slot"));
        elements.push_back(text("  d - Delete save in selected slot"));
        elements.push_back(text("  ESC - Cancel"));

        return vbox(elements) | border;
    });
}

bool SaveLoadScreen::handleInput(const ftxui::Event& event) {
    // Navigation
    if (event == Event::ArrowUp || event == Event::Character('k')) {
        selected_slot = (selected_slot - 1 + MAX_SLOTS) % MAX_SLOTS;
        return true;
    }
    if (event == Event::ArrowDown || event == Event::Character('j')) {
        selected_slot = (selected_slot + 1) % MAX_SLOTS;
        return true;
    }

    // Number keys for direct slot selection
    if (event.is_character()) {
        char c = event.character()[0];
        if (c >= '1' && c <= '9') {
            int slot = c - '0';
            if (slot <= MAX_SLOTS) {
                selected_slot = slot - 1;
                return true;
            }
        }
    }

    // Action
    if (event == Event::Return) {
        int slot = selected_slot + 1;
        if (save_mode) {
            saveToSlot(slot);
        } else {
            if (save_infos[selected_slot].exists) {
                loadFromSlot(slot);
            } else {
                game_manager->getMessageLog()->addMessage("No save in slot " + std::to_string(slot));
            }
        }
        return true;
    }


    // Delete save
    if (event == Event::Character('d') || event == Event::Character('D')) {
        if (save_infos[selected_slot].exists) {
            deleteSlot(selected_slot + 1);
        }
        return true;
    }

    // Cancel
    if (event == Event::Escape) {
        game_manager->setState(GameState::PLAYING);
        return true;
    }

    return false;
}

void SaveLoadScreen::setupMVCCallbacks() {
    // Set up controller -> view callbacks
    controllers::SaveLoadController::ViewCallbacks viewCallbacks;

    viewCallbacks.showMessage = [this](const std::string& msg) {
        if (view) view->showMessage(msg);
        if (game_manager->getMessageLog()) {
            game_manager->getMessageLog()->addMessage(msg);
        }
    };

    viewCallbacks.showError = [this](const std::string& err) {
        if (view) view->showError(err);
        if (game_manager->getMessageLog()) {
            game_manager->getMessageLog()->addMessage("Error: " + err);
        }
    };

    viewCallbacks.updateProgress = [this](float progress) {
        if (view) view->updateProgress(progress);
    };

    viewCallbacks.refreshSlotDisplay = [this]() {
        refreshSaveList();
        if (view) view->refreshSlotDisplay();
    };

    viewCallbacks.exitScreen = [this]() {
        // Return to playing state
        game_manager->setState(GameState::PLAYING);
    };

    viewCallbacks.confirmAction = [this](const std::string& msg) {
        if (view) return view->confirmAction(msg);
        return true;  // Default to true if no view
    };

    controller->setViewCallbacks(viewCallbacks);

    // Set up view -> controller callbacks
    ui::SaveLoadView::ControllerCallbacks controllerCallbacks;

    controllerCallbacks.onSave = [this](int slot) {
        controller->handleSave(slot);
    };

    controllerCallbacks.onLoad = [this](int slot) {
        controller->handleLoad(slot);
    };

    controllerCallbacks.onDelete = [this](int slot) {
        controller->handleDelete(slot);
    };

    controllerCallbacks.onRefresh = [this]() {
        controller->refreshSlots();
    };

    controllerCallbacks.getSaveSlots = [this]() {
        return controller->getSaveSlots();
    };

    controllerCallbacks.canSaveToSlot = [this](int slot) {
        return controller->canSaveToSlot(slot);
    };

    controllerCallbacks.canLoadFromSlot = [this](int slot) {
        return controller->canLoadFromSlot(slot);
    };

    view->setControllerCallbacks(controllerCallbacks);
}