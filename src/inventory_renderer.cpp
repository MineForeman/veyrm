#include "inventory_renderer.h"
#include "player.h"
#include "inventory.h"
#include "item.h"
#include "log.h"
#include <ftxui/dom/elements.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace ftxui;

InventoryRenderer::InventoryRenderer(Player* player)
    : player(player), selected_slot(0), scroll_offset(0) {
}

Element InventoryRenderer::render() {
    if (!player || !player->inventory) {
        return text("No inventory available");
    }

    return window(
        text(" INVENTORY ") | bold,
        vbox({
            renderHeader(),
            separator(),
            renderItemList() | flex,
            separator(),
            renderItemDetails(),
            separator(),
            renderActionBar()
        })
    );
}

Element InventoryRenderer::renderHeader() {
    auto* inventory = player->inventory.get();

    std::stringstream ss;
    ss << " Slots: " << inventory->getUsedSlots() << "/" << inventory->getTotalSlots()
       << " | Weight: " << inventory->getTotalWeight() << " lbs"
       << " | Gold: " << player->gold << " ";
    std::string header = ss.str();

    return text(header) | color(Color::Yellow);
}

Element InventoryRenderer::renderItemList() {
    auto* inventory = player->inventory.get();
    std::vector<Element> items;

    int total_slots = inventory->getTotalSlots();
    auto all_items = inventory->getAllItems();

    // Calculate visible range
    int start = scroll_offset;
    int end = std::min(start + VISIBLE_ROWS, total_slots);

    for (int i = start; i < end; i++) {
        Item* item = (i < static_cast<int>(all_items.size())) ? all_items[i] : nullptr;
        items.push_back(renderSlotLine(i, item));
    }

    // Add scroll indicators if needed
    if (scroll_offset > 0) {
        items.insert(items.begin(), text("  ▲ More items above ▲  ") | dim | center);
    }
    if (end < total_slots) {
        items.push_back(text("  ▼ More items below ▼  ") | dim | center);
    }

    return vbox(std::move(items));
}

Element InventoryRenderer::renderSlotLine(int slot, Item* item) {
    char slot_letter = static_cast<char>('a' + slot);
    std::string line;

    if (item) {
        // Format: "a) Item Name                    x3   5 lb"
        std::string count_str;
        if (item->stackable) {
            std::stringstream ss;
            ss << "x" << item->stack_size;
            count_str = ss.str();
        } else {
            count_str = "  ";
        }

        std::stringstream ss;
        ss << " " << slot_letter << ") "
           << std::left << std::setw(30) << item->name.substr(0, 30)
           << std::right << std::setw(4) << count_str
           << std::setw(4) << (item->weight * (item->stackable ? item->stack_size : 1))
           << " lb ";
        line = ss.str();
    } else {
        std::stringstream ss;
        ss << " " << slot_letter << ") [empty]";
        line = ss.str();
    }

    Element elem = text(line);

    // Apply color based on item type
    if (item) {
        elem = elem | color(getItemColor(item));
    } else {
        elem = elem | dim;
    }

    // Highlight selected slot
    if (slot == selected_slot) {
        elem = elem | inverted;
    }

    return elem;
}

Element InventoryRenderer::renderItemDetails() {
    Item* item = getSelectedItem();

    if (!item) {
        return vbox({
            text(" No item selected ") | dim | center,
            text(""),
            text(" Select an item to see details ") | dim | center
        });
    }

    std::vector<Element> details;
    details.push_back(text(" " + item->name + " ") | bold | color(getItemColor(item)));
    details.push_back(text(""));

    // Type
    std::string type_str = "Unknown";
    switch (item->type) {
        case Item::ItemType::WEAPON: type_str = "Weapon"; break;
        case Item::ItemType::ARMOR: type_str = "Armor"; break;
        case Item::ItemType::POTION: type_str = "Potion"; break;
        case Item::ItemType::SCROLL: type_str = "Scroll"; break;
        case Item::ItemType::FOOD: type_str = "Food"; break;
        case Item::ItemType::GOLD: type_str = "Gold"; break;
        case Item::ItemType::MISC: type_str = "Miscellaneous"; break;
    }
    details.push_back(text(" Type: " + type_str));

    // Weight and value
    std::stringstream weight_ss;
    weight_ss << " Weight: " << item->weight << " lbs";
    details.push_back(text(weight_ss.str()));
    if (item->value > 0) {
        std::stringstream value_ss;
        value_ss << " Value: " << item->value << " gold";
        details.push_back(text(value_ss.str()));
    }

    // Stack info
    if (item->stackable) {
        std::stringstream stack_ss;
        stack_ss << " Stack: " << item->stack_size << "/" << item->max_stack;
        details.push_back(text(stack_ss.str()));
    }

    // Special properties
    if (item->type == Item::ItemType::POTION && item->properties.count("heal")) {
        std::stringstream heal_ss;
        heal_ss << " Healing: " << item->properties.at("heal") << " HP";
        details.push_back(text(heal_ss.str()));
    }

    // Description (if available)
    if (!item->description.empty()) {
        details.push_back(text(""));
        details.push_back(text(" " + item->description) | dim);
    }

    return vbox(std::move(details));
}

Element InventoryRenderer::renderActionBar() {
    Item* item = getSelectedItem();
    std::vector<Element> actions;

    if (item) {
        // Show available actions based on item type
        if (item->type == Item::ItemType::POTION) {
            actions.push_back(text("[u]") | bold);
            actions.push_back(text("se "));
        }

        actions.push_back(text("[D]") | bold);
        actions.push_back(text("rop "));

        actions.push_back(text("[E]") | bold);
        actions.push_back(text("xamine "));
    }

    // Always show close option
    actions.push_back(text("    "));
    actions.push_back(text("[ESC/i]") | bold);
    actions.push_back(text(" close"));

    return hbox(std::move(actions)) | center;
}

void InventoryRenderer::selectNext() {
    auto* inventory = player->inventory.get();
    if (!inventory) return;

    int max_slot = inventory->getTotalSlots() - 1;
    selected_slot = std::min(selected_slot + 1, max_slot);
    ensureSelectionVisible();
}

void InventoryRenderer::selectPrevious() {
    selected_slot = std::max(selected_slot - 1, 0);
    ensureSelectionVisible();
}

void InventoryRenderer::selectSlot(int slot) {
    auto* inventory = player->inventory.get();
    if (!inventory) return;

    selected_slot = std::clamp(slot, 0, inventory->getTotalSlots() - 1);
    ensureSelectionVisible();
}

Item* InventoryRenderer::getSelectedItem() const {
    if (!player || !player->inventory) return nullptr;

    auto* inventory = player->inventory.get();
    return inventory->getItem(selected_slot);
}

void InventoryRenderer::scrollUp() {
    scroll_offset = std::max(scroll_offset - 1, 0);
}

void InventoryRenderer::scrollDown() {
    scroll_offset = std::min(scroll_offset + 1, getMaxScroll());
}

void InventoryRenderer::reset() {
    selected_slot = 0;
    scroll_offset = 0;
}

Color InventoryRenderer::getItemColor(Item* item) const {
    if (!item) return Color::White;

    switch (item->type) {
        case Item::ItemType::WEAPON: return Color::Cyan;
        case Item::ItemType::ARMOR: return Color::Blue;
        case Item::ItemType::POTION: return Color::Magenta;
        case Item::ItemType::SCROLL: return Color::Yellow;
        case Item::ItemType::GOLD: return Color::YellowLight;
        case Item::ItemType::FOOD: return Color::Green;
        default: return Color::White;
    }
}

int InventoryRenderer::getMaxScroll() const {
    if (!player || !player->inventory) return 0;

    auto* inventory = player->inventory.get();
    int total_slots = inventory->getTotalSlots();
    return std::max(0, total_slots - VISIBLE_ROWS);
}

void InventoryRenderer::ensureSelectionVisible() {
    // Adjust scroll to keep selection visible
    if (selected_slot < scroll_offset) {
        scroll_offset = selected_slot;
    } else if (selected_slot >= scroll_offset + VISIBLE_ROWS) {
        scroll_offset = selected_slot - VISIBLE_ROWS + 1;
    }

    // Clamp scroll to valid range
    scroll_offset = std::clamp(scroll_offset, 0, getMaxScroll());
}