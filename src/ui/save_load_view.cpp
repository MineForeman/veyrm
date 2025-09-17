#include "ui/save_load_view.h"
#include "ui/components/list.h"
#include "ui/components/button.h"
#include "ui/components/dialog.h"
#include "ui/components/progress.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <sstream>

using namespace ftxui;

namespace ui {

SaveLoadView::SaveLoadView() {
    // Constructor - initialization done in run()
}

SaveLoadView::Result SaveLoadView::run() {
    // Reset state
    result = Result::CANCELLED;
    selected_slot = 0;
    clearMessages();
    
    // Refresh slots initially
    handleRefreshAction();
    
    // Create main component
    container = createMainComponent();
    
    // Event handler
    auto event_handler = CatchEvent(container, [this](Event event) {
        // Navigation
        if (event == Event::ArrowUp || event == Event::Character('k')) {
            selected_slot = (selected_slot - 1 + max_slots) % max_slots;
            return true;
        }
        if (event == Event::ArrowDown || event == Event::Character('j')) {
            selected_slot = (selected_slot + 1) % max_slots;
            return true;
        }
        
        // Number keys for direct slot selection
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '1' && c <= '9') {
                int slot = c - '0';
                if (slot <= max_slots) {
                    selected_slot = slot - 1;
                    return true;
                }
            }
        }
        
        // Action keys
        if (event == Event::Return) {
            if (current_mode == Mode::SAVE) {
                handleSaveAction();
            } else {
                handleLoadAction();
            }
            return true;
        }
        
        if (event == Event::Character('d') || event == Event::Character('D')) {
            handleDeleteAction();
            return true;
        }
        
        if (event == Event::Character('r') || event == Event::Character('R')) {
            handleRefreshAction();
            return true;
        }
        
        if (event == Event::Escape) {
            exitWithResult(Result::CANCELLED);
            return true;
        }
        
        return false;
    });
    
    screen.Loop(event_handler);
    
    return result;
}

void SaveLoadView::showMessage(const std::string& message) {
    status_message = message;
    show_status = true;
    show_error = false;
}

void SaveLoadView::showError(const std::string& error) {
    error_message = error;
    show_error = true;
    show_status = false;
}

void SaveLoadView::updateProgress(float progress) {
    progress_value = progress;
    show_progress = (progress > 0.0f && progress < 1.0f);
}

void SaveLoadView::refreshSlotDisplay() {
    if (controller_callbacks.getSaveSlots) {
        cached_slots = controller_callbacks.getSaveSlots();
    }
}

void SaveLoadView::exitWithResult(Result exit_result) {
    result = exit_result;
    screen.ExitLoopClosure()();
}

bool SaveLoadView::confirmAction(const std::string& message) {
    confirmation_message = message;
    show_confirmation = true;
    
    // Simple confirmation - in real implementation, would show dialog
    // For now, always return true (should be replaced with proper modal)
    return true;
}

Component SaveLoadView::createMainComponent() {
    return Renderer([this] {
        // Prepare slot items for the list component
        std::vector<ui::components::List::Item> slot_items;

        for (const auto& slot : cached_slots.slots) {
            ui::components::List::Item item;
            item.label = formatSlotDisplay(slot, slot.slot_number - 1);
            item.description = "";
            item.enabled = (current_mode == Mode::SAVE) || slot.exists;
            item.selected = false;
            item.icon = slot.is_cloud_synced ? "☁" : "";
            slot_items.push_back(item);
        }

        // Create list with component
        ui::components::List::Style list_style;
        list_style.selected_color = Color::Cyan;
        list_style.disabled_color = Color::GrayDark;
        list_style.normal_color = Color::White;
        list_style.highlight_color = (current_mode == Mode::SAVE) ? Color::Yellow : Color::Green;
        list_style.show_border = false;

        auto slot_list = ui::components::List::CreateDetailed(
            slot_items,
            selected_slot,
            [this](int /* slot */) {
                if (current_mode == Mode::SAVE) {
                    handleSaveAction();
                } else {
                    handleLoadAction();
                }
            },
            list_style
        );

        Elements elements;

        // Title
        std::string title = (current_mode == Mode::SAVE) ? "SAVE GAME" : "LOAD GAME";
        elements.push_back(
            hbox({
                text(" "),
                text(title) | bold | color(Color::Cyan),
                text(" ")
            }) | center
        );

        elements.push_back(separator());

        // Instructions
        elements.push_back(
            text(current_mode == Mode::SAVE ?
                "Select a slot to save your game:" :
                "Select a slot to load from:") | center
        );

        elements.push_back(text(" "));

        // Add the slot list component
        elements.push_back(slot_list->Render());
        
        elements.push_back(text(" "));
        elements.push_back(separator());
        
        // Status messages
        if (show_error && !error_message.empty()) {
            elements.push_back(
                text(error_message) | color(Color::Red) | center
            );
        }
        
        if (show_status && !status_message.empty()) {
            elements.push_back(
                text(status_message) | color(Color::Green) | center
            );
        }
        
        // Progress bar
        if (show_progress) {
            elements.push_back(createProgressBar()->Render());
        }
        
        // Controls
        elements.push_back(separator());
        elements.push_back(text("Controls:"));
        elements.push_back(text("  ↑/↓ or j/k - Navigate slots"));
        elements.push_back(text("  Enter - " + std::string(current_mode == Mode::SAVE ? "Save" : "Load") + " selected slot"));
        elements.push_back(text("  d - Delete save in selected slot"));
        elements.push_back(text("  r - Refresh slot list"));
        elements.push_back(text("  ESC - Cancel"));
        
        return vbox(elements) | border;
    });
}

Component SaveLoadView::createSlotList() {
    // Slot list component (if needed for more complex layout)
    return Container::Vertical({});
}

Component SaveLoadView::createActionButtons() {
    // Action buttons component (if needed for button-based UI)
    return Container::Horizontal({});
}

Component SaveLoadView::createProgressBar() {
    // Use the Progress component from the UI library
    ui::components::Progress::BarStyle progress_style;
    progress_style.fill_color = Color::Green;
    progress_style.empty_color = Color::GrayDark;
    progress_style.show_percentage = true;
    progress_style.width = 40;
    progress_style.show_border = true;

    return ui::components::Progress::CreateBar(
        progress_value,
        "Saving...",
        progress_style
    );
}

void SaveLoadView::handleSlotSelection(int slot) {
    if (slot >= 0 && slot < max_slots) {
        selected_slot = slot;
    }
}

void SaveLoadView::handleSaveAction() {
    if (controller_callbacks.onSave) {
        int slot_number = selected_slot + 1; // Convert to 1-based
        
        if (controller_callbacks.canSaveToSlot && 
            !controller_callbacks.canSaveToSlot(slot_number)) {
            showError("Cannot save to this slot");
            return;
        }
        
        controller_callbacks.onSave(slot_number);
    }
}

void SaveLoadView::handleLoadAction() {
    if (controller_callbacks.onLoad) {
        int slot_number = selected_slot + 1; // Convert to 1-based
        
        if (controller_callbacks.canLoadFromSlot && 
            !controller_callbacks.canLoadFromSlot(slot_number)) {
            showError("No save in this slot");
            return;
        }
        
        controller_callbacks.onLoad(slot_number);
    }
}

void SaveLoadView::handleDeleteAction() {
    if (controller_callbacks.onDelete) {
        int slot_number = selected_slot + 1; // Convert to 1-based
        
        if (controller_callbacks.canLoadFromSlot && 
            !controller_callbacks.canLoadFromSlot(slot_number)) {
            showError("No save to delete in this slot");
            return;
        }
        
        controller_callbacks.onDelete(slot_number);
    }
}

void SaveLoadView::handleRefreshAction() {
    if (controller_callbacks.onRefresh) {
        controller_callbacks.onRefresh();
    }
    
    if (controller_callbacks.getSaveSlots) {
        cached_slots = controller_callbacks.getSaveSlots();
    }
}

std::string SaveLoadView::formatSlotDisplay(const models::SaveSlot& slot, int index) const {
    std::stringstream ss;
    ss << "Slot " << (index + 1) << ": ";
    
    if (slot.exists) {
        ss << "Lvl " << slot.depth;
        ss << " - " << slot.player_name;
        ss << " (" << slot.player_hp << "/" << slot.player_max_hp << " HP)";
        ss << " - " << slot.timestamp;
    } else {
        ss << "Empty";
    }
    
    return ss.str();
}

void SaveLoadView::clearMessages() {
    status_message.clear();
    error_message.clear();
    confirmation_message.clear();
    show_status = false;
    show_error = false;
    show_progress = false;
    show_confirmation = false;
    progress_value = 0.0f;
}

} // namespace ui