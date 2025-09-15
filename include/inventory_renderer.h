/**
 * @file inventory_renderer.h
 * @brief Inventory UI rendering component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/dom/elements.hpp>
#include <memory>

// Legacy Item class removed - ECS inventory not yet implemented
// class Item;

class InventoryRenderer {
public:
    explicit InventoryRenderer(void* unused_player = nullptr);  // Player removed, parameter kept for compatibility

    // Main render method
    ftxui::Element render();

    // Component renders
    ftxui::Element renderItemList();
    ftxui::Element renderItemDetails();
    ftxui::Element renderActionBar();
    ftxui::Element renderHeader();

    // Selection management
    void selectNext();
    void selectPrevious();
    void selectSlot(int slot);
    int getSelectedSlot() const { return selected_slot; }
    void* getSelectedItem() const;  // Legacy Item* removed

    // Scrolling
    void scrollUp();
    void scrollDown();

    // State
    void reset();

private:
    void* unused_player;  // Player class removed
    int selected_slot;
    int scroll_offset;

    // UI configuration
    static constexpr int VISIBLE_ROWS = 20;
    static constexpr int ITEM_NAME_WIDTH = 30;

    // Helper methods
    ftxui::Element renderSlotLine(int slot, void* item);
    ftxui::Color getItemColor(void* item) const;
    std::string formatItemLine(char slot_letter, void* item) const;
    int getMaxScroll() const;
    void clampSelection();
    void ensureSelectionVisible();
};