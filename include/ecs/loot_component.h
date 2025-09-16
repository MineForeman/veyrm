/**
 * @file loot_component.h
 * @brief Loot table and drop chance component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <vector>
#include <string>
#include <random>

namespace ecs {

/**
 * @struct LootEntry
 * @brief Single loot table entry
 */
struct LootEntry {
    std::string item_id;       ///< Item identifier
    float drop_chance;         ///< Chance to drop (0.0 - 1.0)
    int min_quantity = 1;      ///< Minimum quantity
    int max_quantity = 1;      ///< Maximum quantity
    int min_level = 0;         ///< Minimum player level required

    LootEntry(const std::string& id, float chance, int min_q = 1, int max_q = 1)
        : item_id(id), drop_chance(chance), min_quantity(min_q), max_quantity(max_q) {}
};

/**
 * @class LootComponent
 * @brief Component for entities that drop loot
 */
class LootComponent : public Component<LootComponent> {
public:
    std::vector<LootEntry> loot_table;   ///< Possible drops
    int guaranteed_gold = 0;             ///< Always drops this much gold
    int random_gold_max = 0;             ///< Additional random gold (0 to max)
    float drop_nothing_chance = 0.0f;    ///< Chance to drop nothing at all
    int experience_value = 10;           ///< XP granted when killed

    LootComponent() = default;

    /**
     * @brief Add loot entry
     * @param entry Loot entry to add
     */
    void addLoot(const LootEntry& entry) {
        loot_table.push_back(entry);
    }

    /**
     * @brief Roll for loot drops
     * @param player_level Player's level for level-gated drops
     * @param rng Random number generator
     * @return List of item IDs and quantities
     */
    std::vector<std::pair<std::string, int>> rollLoot(int player_level, std::mt19937& rng) const {
        std::vector<std::pair<std::string, int>> drops;

        // Check for drop nothing
        std::uniform_real_distribution<float> chance_dist(0.0f, 1.0f);
        if (chance_dist(rng) < drop_nothing_chance) {
            return drops;
        }

        // Roll for each loot entry
        for (const auto& entry : loot_table) {
            if (player_level >= entry.min_level) {
                if (chance_dist(rng) <= entry.drop_chance) {
                    std::uniform_int_distribution<int> qty_dist(entry.min_quantity, entry.max_quantity);
                    int quantity = qty_dist(rng);
                    drops.push_back({entry.item_id, quantity});
                }
            }
        }

        return drops;
    }

    /**
     * @brief Get gold drop amount
     * @param rng Random number generator
     * @return Gold amount
     */
    int rollGold(std::mt19937& rng) const {
        int gold = guaranteed_gold;
        if (random_gold_max > 0) {
            std::uniform_int_distribution<int> gold_dist(0, random_gold_max);
            gold += gold_dist(rng);
        }
        return gold;
    }

    std::string getTypeName() const override { return "LootComponent"; }
    ComponentType getType() const override { return ComponentType::CUSTOM; }
};

} // namespace ecs