#include "inventory_renderer.h"
#include "log.h"
#include "ecs/game_world.h"
#include "ecs/entity.h"
#include "ecs/inventory_component.h"
#include "ecs/item_component.h"
#include "ecs/position_component.h"
#include <ftxui/dom/elements.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace ftxui;

InventoryRenderer::InventoryRenderer(void* unused_player)
    : unused_player(unused_player), selected_slot(0), scroll_offset(0) {
}

Element InventoryRenderer::render() {
    // Update inventory items from ECS
    inventory_items.clear();

    if (ecs_world) {
        auto player_id = ecs_world->getPlayerID();
        auto* player = ecs_world->getEntity(player_id);

        if (player) {
            auto* inv_comp = player->getComponent<ecs::InventoryComponent>();
            if (inv_comp) {
                // Get all items in inventory
                for (auto item_id : inv_comp->items) {
                    auto* item_entity = ecs_world->getEntity(item_id);
                    if (item_entity) {
                        inventory_items.push_back(item_entity);
                    }
                }
            }
        }
    }

    if (inventory_items.empty()) {
        return window(
            text(" INVENTORY ") | bold,
            vbox({
                text(" Your inventory is empty ") | center | dim,
                separator(),
                renderActionBar()
            })
        );
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
    if (ecs_world) {
        auto player_id = ecs_world->getPlayerID();
        auto* player = ecs_world->getEntity(player_id);

        if (player) {
            auto* inv_comp = player->getComponent<ecs::InventoryComponent>();
            if (inv_comp) {
                std::stringstream ss;
                ss << " Items: " << inv_comp->items.size()
                   << "/" << inv_comp->max_capacity
                   << "  Weight: " << std::fixed << std::setprecision(1)
                   << inv_comp->current_weight << "/" << inv_comp->max_weight << " ";
                return text(ss.str()) | color(Color::Yellow);
            }
        }
    }
    return text(" Inventory ") | color(Color::Yellow);
}

Element InventoryRenderer::renderItemList() {
    std::vector<Element> items;

    int start = scroll_offset;
    int end = std::min(start + VISIBLE_ROWS, static_cast<int>(inventory_items.size()));

    for (int i = start; i < end; ++i) {
        items.push_back(renderSlotLine(i, inventory_items[i]));
    }

    // Fill remaining slots with empty
    for (int i = end - start; i < VISIBLE_ROWS; ++i) {
        char slot_letter = static_cast<char>('a' + i + start);
        if (i + start < 26) {  // Only show slots a-z
            std::stringstream ss;
            ss << " " << slot_letter << ") [empty]";
            items.push_back(text(ss.str()) | dim);
        }
    }

    return vbox(std::move(items));
}

Element InventoryRenderer::renderSlotLine(int slot, void* item_ptr) {
    char slot_letter = static_cast<char>('a' + slot);

    if (!item_ptr) {
        std::stringstream ss;
        ss << " " << slot_letter << ") [empty]";
        return text(ss.str()) | dim;
    }

    auto* item = static_cast<ecs::Entity*>(item_ptr);
    auto* item_comp = item->getComponent<ecs::ItemComponent>();

    if (!item_comp) {
        std::stringstream ss;
        ss << " " << slot_letter << ") [invalid item]";
        return text(ss.str()) | dim;
    }

    std::stringstream ss;
    ss << " " << slot_letter << ") " << std::left << std::setw(30) << item_comp->name;

    if (item_comp->stack_size > 1) {
        ss << " x" << item_comp->stack_size;
    }

    ss << std::right << std::setw(6) << std::fixed << std::setprecision(1)
       << item_comp->weight << " lb";

    Element elem = text(ss.str());

    // Apply color based on item type
    Color item_color = getItemColor(item_ptr);
    elem = elem | color(item_color);

    // Highlight selected slot
    if (slot == selected_slot) {
        elem = elem | inverted;
    }

    return elem;
}

Element InventoryRenderer::renderItemDetails() {
    if (selected_slot >= static_cast<int>(inventory_items.size())) {
        return vbox({
            text(" No item selected ") | bold,
            separator(),
            text(" Select an item to see details ") | dim
        });
    }

    auto* item = inventory_items[selected_slot];
    auto* item_comp = item->getComponent<ecs::ItemComponent>();

    if (!item_comp) {
        return text(" No details available ") | dim;
    }

    std::vector<Element> details;

    // Item name
    details.push_back(text(" " + item_comp->name + " ") | bold | color(getItemColor(item)));
    details.push_back(separator());

    // Description
    if (!item_comp->description.empty()) {
        details.push_back(text(" " + item_comp->description) | dim);
        details.push_back(separator());
    }

    // Properties
    std::stringstream props;
    props << " Type: ";
    switch (item_comp->item_type) {
        case ecs::ItemType::WEAPON: props << "Weapon"; break;
        case ecs::ItemType::ARMOR: props << "Armor"; break;
        case ecs::ItemType::CONSUMABLE: props << "Consumable"; break;
        case ecs::ItemType::POTION: props << "Potion"; break;
        case ecs::ItemType::SCROLL: props << "Scroll"; break;
        case ecs::ItemType::FOOD: props << "Food"; break;
        default: props << "Misc"; break;
    }

    props << "  Value: " << item_comp->value << " gold";
    details.push_back(text(props.str()));

    // Special properties
    if (item_comp->heal_amount > 0) {
        details.push_back(text(" Heals: " + std::to_string(item_comp->heal_amount) + " HP") | color(Color::Green));
    }

    if (item_comp->damage_bonus > 0) {
        details.push_back(text(" Damage: +" + std::to_string(item_comp->damage_bonus)) | color(Color::Red));
    }

    if (item_comp->defense_bonus > 0) {
        details.push_back(text(" Defense: +" + std::to_string(item_comp->defense_bonus)) | color(Color::Blue));
    }

    return vbox(std::move(details));
}

Element InventoryRenderer::renderActionBar() {
    return hbox({
        text(" [u] Use ") | bold,
        text(" [d] Drop ") | bold,
        text(" [x] Examine ") | bold,
        text(" [ESC] Back ") | bold
    }) | center;
}

void InventoryRenderer::selectNext() {
    if (!inventory_items.empty()) {
        selected_slot = std::min(selected_slot + 1, static_cast<int>(inventory_items.size()) - 1);
        ensureSelectionVisible();
    }
}

void InventoryRenderer::selectPrevious() {
    selected_slot = std::max(selected_slot - 1, 0);
    ensureSelectionVisible();
}

void InventoryRenderer::selectSlot(int slot) {
    if (slot >= 0 && slot < static_cast<int>(inventory_items.size())) {
        selected_slot = slot;
        ensureSelectionVisible();
    }
}

void* InventoryRenderer::getSelectedItem() const {
    if (selected_slot >= 0 && selected_slot < static_cast<int>(inventory_items.size())) {
        return inventory_items[selected_slot];
    }
    return nullptr;
}

void InventoryRenderer::scrollUp() {
    scroll_offset = std::max(0, scroll_offset - 1);
}

void InventoryRenderer::scrollDown() {
    int max_scroll = getMaxScroll();
    scroll_offset = std::min(max_scroll, scroll_offset + 1);
}

void InventoryRenderer::reset() {
    selected_slot = 0;
    scroll_offset = 0;
    inventory_items.clear();
}

Color InventoryRenderer::getItemColor(void* item_ptr) const {
    if (!item_ptr) return Color::White;

    auto* item = static_cast<ecs::Entity*>(item_ptr);
    auto* item_comp = item->getComponent<ecs::ItemComponent>();

    if (!item_comp) return Color::White;

    switch (item_comp->item_type) {
        case ecs::ItemType::WEAPON: return Color::Cyan;
        case ecs::ItemType::ARMOR: return Color::Blue;
        case ecs::ItemType::POTION: return Color::Magenta;
        case ecs::ItemType::SCROLL: return Color::Yellow;
        case ecs::ItemType::FOOD: return Color::Green;
        case ecs::ItemType::CONSUMABLE: return Color::GreenLight;
        default: return Color::White;
    }
}

std::string InventoryRenderer::formatItemLine(char slot_letter, void* item_ptr) const {
    if (!item_ptr) {
        return std::string(1, slot_letter) + ") [empty]";
    }

    auto* item = static_cast<ecs::Entity*>(item_ptr);
    auto* item_comp = item->getComponent<ecs::ItemComponent>();

    if (item_comp) {
        return std::string(1, slot_letter) + ") " + item_comp->name;
    }

    return std::string(1, slot_letter) + ") [unknown]";
}

int InventoryRenderer::getMaxScroll() const {
    int total_items = static_cast<int>(inventory_items.size());
    return std::max(0, total_items - VISIBLE_ROWS);
}

void InventoryRenderer::ensureSelectionVisible() {
    if (selected_slot < scroll_offset) {
        scroll_offset = selected_slot;
    } else if (selected_slot >= scroll_offset + VISIBLE_ROWS) {
        scroll_offset = selected_slot - VISIBLE_ROWS + 1;
    }
}

void InventoryRenderer::clampSelection() {
    int total_items = static_cast<int>(inventory_items.size());
    if (total_items > 0 && selected_slot >= total_items) {
        selected_slot = total_items - 1;
    } else if (total_items == 0) {
        selected_slot = 0;
    }
}