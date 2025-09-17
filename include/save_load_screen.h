/**
 * @file save_load_screen.h
 * @brief Save/load game screen UI component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>
#include <memory>
#include "game_state.h"
#include "serializable.h"

// Forward declarations
class GameManager;
class GameSerializer;

namespace controllers {
    class SaveLoadController;
}

namespace services {
    class SaveGameService;
}

namespace ui {
    class SaveLoadView;
}

class SaveLoadScreen {
public:
    explicit SaveLoadScreen(GameManager* game_manager);
    ~SaveLoadScreen();  // Destructor must be defined in .cpp for unique_ptr with forward declaration

    // Create the UI component
    ftxui::Component create();

    // Handle input
    bool handleInput(const ftxui::Event& event);

    // UI state
    void reset();
    void refreshSaveList();
    void setSaveMode(bool save) { save_mode = save; }

private:
    // Create menu options
    std::vector<std::string> getSaveSlotDescriptions() const;
    std::string formatSaveInfo(int slot) const;

    // Actions
    void saveToSlot(int slot);
    void loadFromSlot(int slot);
    void deleteSlot(int slot);

    // Setup MVC callbacks
    void setupMVCCallbacks();

    // State
    GameManager* game_manager;
    int selected_slot = 0;
    bool save_mode = true;  // true = save, false = load
    std::vector<SaveInfo> save_infos;

    // MVC Components
    std::shared_ptr<services::SaveGameService> save_service;
    std::unique_ptr<controllers::SaveLoadController> controller;
    std::unique_ptr<ui::SaveLoadView> view;

    // UI Constants
    static constexpr int MAX_SLOTS = 9;
};