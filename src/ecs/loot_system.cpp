/**
 * @file loot_system.cpp
 * @brief Implementation of loot system
 */

#include <sstream>
#include <chrono>
#include "ecs/loot_system.h"
#include "ecs/item_component.h"
#include "ecs/renderable_component.h"
#include "map.h"

namespace ecs {

LootSystem::LootSystem(Map* map, ILogger* logger)
    : map(map)
    , logger(logger) {
    // Seed RNG with current time
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    rng.seed(static_cast<unsigned int>(seed));
}

void LootSystem::update(const std::vector<std::unique_ptr<Entity>>&, double) {
    // Loot system is mostly event-driven, not much to do in update
}

std::vector<std::unique_ptr<Entity>> LootSystem::generateLoot(Entity* entity, int killer_level) {
    std::vector<std::unique_ptr<Entity>> drops;

    if (!entity) return drops;

    auto* loot = entity->getComponent<LootComponent>();
    auto* pos = entity->getComponent<PositionComponent>();

    if (!loot || !pos) return drops;

    return dropLoot(*loot, pos->position.x, pos->position.y, killer_level);
}

std::vector<std::unique_ptr<Entity>> LootSystem::dropLoot(const LootComponent& loot,
                                                          int x, int y, int killer_level) {
    std::vector<std::unique_ptr<Entity>> drops;

    // Roll for each loot entry
    auto rolled_items = loot.rollLoot(killer_level, rng);
    for (const auto& [item_id, quantity] : rolled_items) {
        auto [drop_x, drop_y] = getRandomNearbyPosition(x, y);
        auto item = createItemFromId(item_id, quantity, drop_x, drop_y);
        if (item) {
            drops.push_back(std::move(item));
        }
    }

    // Drop gold
    int gold = loot.rollGold(rng);
    if (gold > 0) {
        auto [drop_x, drop_y] = getRandomNearbyPosition(x, y);
        auto gold_pile = createGold(gold, drop_x, drop_y);
        if (gold_pile) {
            drops.push_back(std::move(gold_pile));
        }
    }

    if (logger && !drops.empty()) {
        std::stringstream msg;
        msg << "Dropped " << drops.size() << " items";
        logger->logSystem(msg.str());
    }

    return drops;
}

std::unique_ptr<Entity> LootSystem::createTreasureChest(int x, int y, int treasure_level) {
    auto chest = std::make_unique<Entity>();

    // Add position
    chest->addComponent<PositionComponent>(x, y);

    // Add renderable
    auto& renderable = chest->addComponent<RenderableComponent>();
    renderable.glyph = '=';
    renderable.name = "Treasure Chest";
    renderable.color = {255, 215, 0};  // Gold color

    // Add loot component with treasure
    auto& loot = chest->addComponent<LootComponent>();
    loot.guaranteed_gold = treasure_level * 50;
    loot.random_gold_max = treasure_level * 100;

    // Add items based on treasure level
    if (treasure_level >= 1) {
        loot.addLoot(LootEntry("potion_health", 0.7f, 1, 2));
    }
    if (treasure_level >= 2) {
        loot.addLoot(LootEntry("potion_mana", 0.5f, 1, 2));
        loot.addLoot(LootEntry("scroll_teleport", 0.3f));
    }
    if (treasure_level >= 3) {
        loot.addLoot(LootEntry("weapon_magic", 0.2f));
        loot.addLoot(LootEntry("armor_magic", 0.2f));
    }

    // Tag as chest
    chest->addTag("chest");
    chest->addTag("container");

    return chest;
}

std::vector<std::unique_ptr<Entity>> LootSystem::openChest(Entity* chest) {
    std::vector<std::unique_ptr<Entity>> drops;

    if (!chest || !chest->hasTag("chest")) return drops;

    auto* loot = chest->getComponent<LootComponent>();
    auto* pos = chest->getComponent<PositionComponent>();

    if (!loot || !pos) return drops;

    // Generate loot
    drops = dropLoot(*loot, pos->position.x, pos->position.y, 999);  // High level for all drops

    if (logger) {
        auto* renderable = chest->getComponent<RenderableComponent>();
        std::string name = renderable ? renderable->name : "Chest";
        std::stringstream msg;
        msg << name << " opened, found " << drops.size() << " items";
        logger->logSystem(msg.str());
    }

    // Mark chest as opened
    chest->addTag("opened");

    // Change appearance
    auto* renderable = chest->getComponent<RenderableComponent>();
    if (renderable) {
        renderable->glyph = '[';
        renderable->name = "Empty Chest";
        renderable->color = {128, 128, 128};  // Gray
    }

    return drops;
}

std::unique_ptr<Entity> LootSystem::createItemFromId(const std::string& item_id,
                                                     int quantity, int x, int y) {
    auto item = std::make_unique<Entity>();

    // Add position
    item->addComponent<PositionComponent>(x, y);

    // Add item component
    auto& item_comp = item->addComponent<ItemComponent>();
    item_comp.name = item_id;  // Would normally look up from item database
    item_comp.stack_size = quantity;

    // Add renderable
    auto& renderable = item->addComponent<RenderableComponent>();

    // Set appearance based on item type (simplified)
    if (item_id.find("potion") != std::string::npos) {
        renderable.glyph = '!';
        renderable.color = {255, 0, 255};  // Magenta
        item_comp.item_type = ItemType::POTION;
        item_comp.consumable = true;
        item_comp.heal_amount = 20;
    } else if (item_id.find("scroll") != std::string::npos) {
        renderable.glyph = '?';
        renderable.color = {255, 255, 0};  // Yellow
        item_comp.item_type = ItemType::SCROLL;
        item_comp.consumable = true;
    } else if (item_id.find("weapon") != std::string::npos) {
        renderable.glyph = '/';
        renderable.color = {192, 192, 192};  // Silver
        item_comp.item_type = ItemType::WEAPON;
        item_comp.equippable = true;
        item_comp.attack_bonus = 2;
        item_comp.damage_bonus = 3;
    } else if (item_id.find("armor") != std::string::npos) {
        renderable.glyph = '[';
        renderable.color = {128, 128, 255};  // Light blue
        item_comp.item_type = ItemType::ARMOR;
        item_comp.equippable = true;
        item_comp.defense_bonus = 5;
    } else {
        renderable.glyph = '*';
        renderable.color = {255, 255, 255};  // White
        item_comp.item_type = ItemType::MISC;
    }

    renderable.name = item_id;

    // Tag as item
    item->addTag("item");

    return item;
}

std::unique_ptr<Entity> LootSystem::createGold(int amount, int x, int y) {
    auto gold = std::make_unique<Entity>();

    // Add position
    gold->addComponent<PositionComponent>(x, y);

    // Add item component
    auto& item = gold->addComponent<ItemComponent>();
    item.name = "Gold";
    item.value = amount;
    item.stack_size = amount;
    item.max_stack = 99999;
    item.weight = 0.01f;  // Gold is light per coin

    // Add renderable
    auto& renderable = gold->addComponent<RenderableComponent>();
    renderable.glyph = '$';
    renderable.name = std::to_string(amount) + " gold";
    renderable.color = {255, 215, 0};  // Gold color

    // Tag as gold/currency
    gold->addTag("gold");
    gold->addTag("currency");
    gold->addTag("item");

    return gold;
}

std::pair<int, int> LootSystem::getRandomNearbyPosition(int center_x, int center_y, int radius) {
    std::uniform_int_distribution<int> offset_dist(-radius, radius);

    // Try to find a valid position
    for (int attempts = 0; attempts < 10; attempts++) {
        int x = center_x + offset_dist(rng);
        int y = center_y + offset_dist(rng);

        // Check if position is valid (would need map validation)
        if (map && map->isWalkable(x, y)) {
            return {x, y};
        }
    }

    // Fallback to center position
    return {center_x, center_y};
}

} // namespace ecs