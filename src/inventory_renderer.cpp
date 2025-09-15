#include "inventory_renderer.h"
// #include "inventory.h"  // Legacy - removed
// #include "item.h"  // Legacy - removed
#include "log.h"
#include <ftxui/dom/elements.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace ftxui;

InventoryRenderer::InventoryRenderer(void* unused_player)
    : unused_player(unused_player), selected_slot(0), scroll_offset(0) {
}

Element InventoryRenderer::render() {
    // Legacy inventory system removed - ECS inventory not yet implemented
    return text("Inventory system is being migrated to ECS");
}

Element InventoryRenderer::renderHeader() {
    // Legacy inventory system removed
    return text(" Inventory: Migrating to ECS ") | color(Color::Yellow);
}

Element InventoryRenderer::renderItemList() {
    // Legacy inventory system removed
    return text("[Inventory migrating to ECS]");
}

Element InventoryRenderer::renderSlotLine(int slot, void* /*item*/) {
    // Legacy inventory system removed
    char slot_letter = static_cast<char>('a' + slot);
    std::stringstream ss;
    ss << " " << slot_letter << ") [ECS migration in progress]";
    return text(ss.str()) | dim;
}

Element InventoryRenderer::renderItemDetails() {
    // Legacy inventory system removed
    return vbox({
        text(" Item Details ") | bold,
        separator(),
        text(" ECS migration in progress ") | dim
    });
}

Element InventoryRenderer::renderActionBar() {
    // Legacy inventory system removed
    return hbox({
        text(" [ECS Migration] ") | dim,
        text(" ESC: Back ") | bold
    }) | center;
}

void InventoryRenderer::selectNext() {
    // Legacy inventory system removed
    selected_slot = std::min(selected_slot + 1, 25);
}

void InventoryRenderer::selectPrevious() {
    // Legacy inventory system removed
    selected_slot = std::max(selected_slot - 1, 0);
}

void InventoryRenderer::selectSlot(int slot) {
    // Legacy inventory system removed
    if (slot >= 0 && slot < 26) {
        selected_slot = slot;
    }
}

void* InventoryRenderer::getSelectedItem() const {
    // Legacy inventory system removed
    return nullptr;
}

void InventoryRenderer::scrollUp() {
    // Legacy inventory system removed
    scroll_offset = std::max(0, scroll_offset - 1);
}

void InventoryRenderer::scrollDown() {
    // Legacy inventory system removed
    int max_scroll = getMaxScroll();
    scroll_offset = std::min(max_scroll, scroll_offset + 1);
}

void InventoryRenderer::reset() {
    // Legacy inventory system removed
    selected_slot = 0;
    scroll_offset = 0;
}

Color InventoryRenderer::getItemColor(void* /*item*/) const {
    // Legacy Item type checking removed
    return Color::White;
}

std::string InventoryRenderer::formatItemLine(char slot_letter, void* /*item*/) const {
    // Legacy inventory system removed
    return std::string(1, slot_letter) + ") [ECS migration]";
}

int InventoryRenderer::getMaxScroll() const {
    // Legacy inventory system removed
    int total_slots = 26;  // Default slots
    return std::max(0, total_slots - VISIBLE_ROWS);
}

void InventoryRenderer::ensureSelectionVisible() {
    // Adjust scroll to keep selection visible
    if (selected_slot < scroll_offset) {
        scroll_offset = selected_slot;
    } else if (selected_slot >= scroll_offset + VISIBLE_ROWS) {
        scroll_offset = selected_slot - VISIBLE_ROWS + 1;
    }
}

void InventoryRenderer::clampSelection() {
    // Legacy inventory system removed
    int total_items = 0;  // No items in legacy system
    if (selected_slot >= total_items) {
        selected_slot = std::max(0, total_items - 1);
    }
}