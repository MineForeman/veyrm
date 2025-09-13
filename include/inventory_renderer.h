#pragma once

#include <ftxui/dom/elements.hpp>
#include <memory>

class Player;
class Item;

class InventoryRenderer {
public:
    explicit InventoryRenderer(Player* player);

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
    Item* getSelectedItem() const;

    // Scrolling
    void scrollUp();
    void scrollDown();

    // State
    void reset();

private:
    Player* player;
    int selected_slot;
    int scroll_offset;

    // UI configuration
    static constexpr int VISIBLE_ROWS = 20;
    static constexpr int ITEM_NAME_WIDTH = 30;

    // Helper methods
    ftxui::Element renderSlotLine(int slot, Item* item);
    ftxui::Color getItemColor(Item* item) const;
    std::string formatItemLine(char slot_letter, Item* item) const;
    int getMaxScroll() const;
    void clampSelection();
    void ensureSelectionVisible();
};