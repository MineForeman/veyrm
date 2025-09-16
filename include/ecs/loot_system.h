/**
 * @file loot_system.h
 * @brief System for managing loot drops and treasure generation
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "entity.h"
#include "loot_component.h"
#include "position_component.h"
#include "logger_interface.h"
#include <memory>
#include <random>
#include <vector>

// Forward declarations
class Map;

namespace ecs {

/**
 * @class LootSystem
 * @brief Manages loot drops from entities
 */
class LootSystem : public System<LootSystem> {
public:
    LootSystem(Map* map, ILogger* logger = nullptr);
    ~LootSystem() = default;

    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Generate loot drops from an entity
     * @param entity Entity dropping loot
     * @param killer_level Level of the killer (for level-gated drops)
     * @return List of created item entities
     */
    std::vector<std::unique_ptr<Entity>> generateLoot(Entity* entity, int killer_level = 1);

    /**
     * @brief Drop loot at a position
     * @param loot Loot component
     * @param x X position
     * @param y Y position
     * @param killer_level Level of killer
     * @return List of created item entities
     */
    std::vector<std::unique_ptr<Entity>> dropLoot(const LootComponent& loot,
                                                  int x, int y, int killer_level = 1);

    /**
     * @brief Create a treasure chest at position
     * @param x X position
     * @param y Y position
     * @param treasure_level Level of treasure
     * @return Chest entity
     */
    std::unique_ptr<Entity> createTreasureChest(int x, int y, int treasure_level = 1);

    /**
     * @brief Open a treasure chest
     * @param chest Chest entity
     * @return List of created item entities
     */
    std::vector<std::unique_ptr<Entity>> openChest(Entity* chest);

    int getPriority() const override { return 45; }

    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<LootComponent>();
    }

private:
    Map* map;
    ILogger* logger;
    std::mt19937 rng;

    /**
     * @brief Create item entity from ID
     * @param item_id Item identifier
     * @param quantity Stack size
     * @param x X position
     * @param y Y position
     * @return Item entity
     */
    std::unique_ptr<Entity> createItemFromId(const std::string& item_id,
                                            int quantity, int x, int y);

    /**
     * @brief Create gold pile entity
     * @param amount Gold amount
     * @param x X position
     * @param y Y position
     * @return Gold entity
     */
    std::unique_ptr<Entity> createGold(int amount, int x, int y);

    /**
     * @brief Get random position near target
     * @param center_x Center X
     * @param center_y Center Y
     * @param radius Max distance
     * @return Position pair
     */
    std::pair<int, int> getRandomNearbyPosition(int center_x, int center_y, int radius = 1);
};

} // namespace ecs