#include "save_load_screen.h"
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
    refreshSaveList();
}

void SaveLoadScreen::reset() {
    selected_slot = 0;
    refreshSaveList();

    // Get the mode from GameManager
    save_mode = game_manager->getSaveMenuMode();
}

void SaveLoadScreen::refreshSaveList() {
    save_infos.clear();
    if (!game_manager->getSerializer()) return;

    for (int i = 1; i <= MAX_SLOTS; i++) {
        save_infos.push_back(game_manager->getSerializer()->getSaveInfo(
            game_manager->getSerializer()->getSlotFilename(i)));
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
    if (game_manager->saveGame(slot)) {
        game_manager->getMessageLog()->addMessage("Game saved to slot " + std::to_string(slot));
        game_manager->setState(GameState::PLAYING);
    } else {
        game_manager->getMessageLog()->addMessage("Failed to save game!");
    }
}

void SaveLoadScreen::loadFromSlot(int slot) {
    if (game_manager->loadGame(slot)) {
        game_manager->getMessageLog()->addMessage("Game loaded from slot " + std::to_string(slot));
        game_manager->setState(GameState::PLAYING);
    } else {
        game_manager->getMessageLog()->addMessage("Failed to load game!");
    }
}

void SaveLoadScreen::deleteSlot(int slot) {
    if (game_manager->getSerializer() &&
        game_manager->getSerializer()->deleteSave(slot)) {
        game_manager->getMessageLog()->addMessage("Save deleted from slot " + std::to_string(slot));
        refreshSaveList();
    }
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