/**
 * @file entity_factory.cpp
 * @brief Implementation of entity factory
 */

#include <random>
#include <chrono>
#include "ecs/entity_factory.h"
#include "ecs/data_loader.h"
#include "ecs/item_component.h"
#include "ecs/effects_component.h"
#include "ecs/stats_component.h"
#include "ecs/inventory_component.h"
#include "ecs/equipment_component.h"
#include "ecs/experience_component.h"
#include "ecs/loot_component.h"

namespace ecs {

std::unique_ptr<Entity> EntityFactory::createPlayer(int x, int y, const std::string& name) {
    auto player = std::make_unique<Entity>();

    // Position
    player->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = player->addComponent<RenderableComponent>();
    renderable.glyph = '@';
    renderable.name = name;
    renderable.color = {255, 255, 255};

    // Health
    auto& health = player->addComponent<HealthComponent>();
    health.max_hp = 100;
    health.hp = 100;

    // Stats
    auto& stats = player->addComponent<StatsComponent>();
    stats.strength = 14;
    stats.dexterity = 12;
    stats.intelligence = 10;
    stats.constitution = 14;
    stats.wisdom = 10;
    stats.charisma = 10;
    stats.recalculateDerived();

    // Combat
    auto& combat = player->addComponent<CombatComponent>();
    combat.min_damage = 1;
    combat.max_damage = 6;
    combat.attack_bonus = stats.accuracy_bonus;
    combat.damage_modifier = stats.damage_bonus;
    combat.defense_bonus = 2;  // Base defense

    // Inventory
    player->addComponent<InventoryComponent>();

    // Equipment
    player->addComponent<EquipmentComponent>();

    // Experience
    player->addComponent<ExperienceComponent>();

    // Tags
    player->addTag("player");
    player->addTag("humanoid");

    return player;
}

std::unique_ptr<Entity> EntityFactory::createMonster(const std::string& monster_id,
                                                     int x, int y, int level) {
    auto monster = std::make_unique<Entity>();

    // Position
    monster->addComponent<PositionComponent>(x, y);

    // Apply template
    applyMonsterTemplate(monster.get(), monster_id, level);

    // Tags
    monster->addTag("monster");
    monster->addTag("hostile");

    return monster;
}

std::unique_ptr<Entity> EntityFactory::createItem(const std::string& item_id,
                                                  int x, int y, int quantity) {
    auto item = std::make_unique<Entity>();

    // Position
    item->addComponent<PositionComponent>(x, y);

    // Apply template
    applyItemTemplate(item.get(), item_id, quantity);

    // Tags
    item->addTag("item");

    return item;
}

std::unique_ptr<Entity> EntityFactory::createNPC(const std::string& npc_id,
                                                 int x, int y,
                                                 const std::string& dialogue_id) {
    auto npc = std::make_unique<Entity>();

    // Position
    npc->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = npc->addComponent<RenderableComponent>();
    renderable.glyph = 'N';
    renderable.name = npc_id;
    renderable.color = {200, 200, 255};

    // Health (non-hostile NPCs still have health)
    auto& health = npc->addComponent<HealthComponent>();
    health.max_hp = 50;
    health.hp = 50;

    // AI (peaceful)
    auto& ai = npc->addComponent<AIComponent>();
    ai.behavior = AIBehavior::PASSIVE;

    // Tags
    npc->addTag("npc");
    npc->addTag("friendly");
    if (!dialogue_id.empty()) {
        npc->addTag("dialogue:" + dialogue_id);
    }

    return npc;
}

std::unique_ptr<Entity> EntityFactory::createDoor(int x, int y, bool locked,
                                                  const std::string& key_id) {
    auto door = std::make_unique<Entity>();

    // Position
    door->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = door->addComponent<RenderableComponent>();
    renderable.glyph = locked ? '+' : '-';
    renderable.name = locked ? "Locked Door" : "Door";
    renderable.color = {139, 69, 19};  // Brown

    // Tags
    door->addTag("door");
    if (locked) {
        door->addTag("locked");
        if (!key_id.empty()) {
            door->addTag("key:" + key_id);
        }
    } else {
        door->addTag("closed");
    }

    return door;
}

std::unique_ptr<Entity> EntityFactory::createContainer(int x, int y,
                                                       const std::string& container_type,
                                                       bool locked) {
    auto container = std::make_unique<Entity>();

    // Position
    container->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = container->addComponent<RenderableComponent>();
    if (container_type == "chest") {
        renderable.glyph = '=';
        renderable.name = locked ? "Locked Chest" : "Chest";
        renderable.color = {184, 134, 11};  // Dark golden
    } else if (container_type == "barrel") {
        renderable.glyph = 'o';
        renderable.name = "Barrel";
        renderable.color = {139, 90, 43};  // Brown
    } else {
        renderable.glyph = '&';
        renderable.name = "Container";
        renderable.color = {128, 128, 128};
    }

    // Inventory for storing items
    container->addComponent<InventoryComponent>();

    // Tags
    container->addTag("container");
    container->addTag(container_type);
    if (locked) {
        container->addTag("locked");
    }

    return container;
}

std::unique_ptr<Entity> EntityFactory::createTrap(int x, int y,
                                                  const std::string& trap_type,
                                                  int damage) {
    auto trap = std::make_unique<Entity>();

    // Position
    trap->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = trap->addComponent<RenderableComponent>();
    renderable.glyph = '^';
    renderable.name = trap_type + " trap";
    renderable.color = {128, 128, 128};  // Gray, hard to see

    // Combat component for damage
    auto& combat = trap->addComponent<CombatComponent>();
    combat.min_damage = damage;
    combat.max_damage = damage * 2;

    // Tags
    trap->addTag("trap");
    trap->addTag("hidden");
    trap->addTag(trap_type);

    return trap;
}

std::unique_ptr<Entity> EntityFactory::createStairs(int x, int y,
                                                    bool going_down,
                                                    int destination_level) {
    auto stairs = std::make_unique<Entity>();

    // Position
    stairs->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = stairs->addComponent<RenderableComponent>();
    renderable.glyph = going_down ? '>' : '<';
    renderable.name = going_down ? "Stairs Down" : "Stairs Up";
    renderable.color = {192, 192, 192};  // Silver

    // Tags
    stairs->addTag("stairs");
    stairs->addTag(going_down ? "down" : "up");
    if (destination_level >= 0) {
        stairs->addTag("level:" + std::to_string(destination_level));
    }

    return stairs;
}

std::unique_ptr<Entity> EntityFactory::createLight(int x, int y, int radius,
                                                   const Color& color) {
    auto light = std::make_unique<Entity>();

    // Position
    light->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = light->addComponent<RenderableComponent>();
    renderable.glyph = '*';
    renderable.name = "Light";
    renderable.color = color;

    // Tags
    light->addTag("light");
    light->addTag("radius:" + std::to_string(radius));

    return light;
}

std::unique_ptr<Entity> EntityFactory::createProjectile(int x, int y,
                                                        int target_x, int target_y,
                                                        int damage, float speed) {
    auto projectile = std::make_unique<Entity>();

    // Position
    projectile->addComponent<PositionComponent>(x, y);

    // Renderable
    auto& renderable = projectile->addComponent<RenderableComponent>();

    // Determine projectile glyph based on direction
    int dx = target_x - x;
    int dy = target_y - y;
    if (std::abs(dx) > std::abs(dy)) {
        renderable.glyph = '-';  // Horizontal
    } else if (std::abs(dy) > std::abs(dx)) {
        renderable.glyph = '|';  // Vertical
    } else {
        renderable.glyph = '*';  // Diagonal
    }

    renderable.name = "Projectile";
    renderable.color = {255, 255, 0};  // Yellow

    // Combat for damage
    auto& combat = projectile->addComponent<CombatComponent>();
    combat.min_damage = damage;
    combat.max_damage = damage;

    // Tags
    projectile->addTag("projectile");
    projectile->addTag("target:" + std::to_string(target_x) + "," + std::to_string(target_y));
    projectile->addTag("speed:" + std::to_string(speed));

    return projectile;
}

std::unique_ptr<Entity> EntityFactory::createFromJSON(const boost::json::value& json) {
    auto entity = std::make_unique<Entity>();

    // Parse position
    if (json.is_object() && json.as_object().contains("position")) {
        const auto& pos_obj = json.as_object().at("position");
        if (pos_obj.is_object()) {
            const auto& pos = pos_obj.as_object();
            int x = pos.contains("x") ? boost::json::value_to<int>(pos.at("x")) : 0;
            int y = pos.contains("y") ? boost::json::value_to<int>(pos.at("y")) : 0;
            entity->addComponent<PositionComponent>(x, y);
        }
    }

    // Parse renderable
    if (json.is_object() && json.as_object().contains("renderable")) {
        const auto& rend_obj = json.as_object().at("renderable");
        if (rend_obj.is_object()) {
            auto& renderable = entity->addComponent<RenderableComponent>();
            const auto& rend = rend_obj.as_object();
            if (rend.contains("glyph")) {
                std::string glyph_str = boost::json::value_to<std::string>(rend.at("glyph"));
                renderable.glyph = glyph_str.empty() ? '?' : glyph_str[0];
            }
            if (rend.contains("name")) {
                renderable.name = boost::json::value_to<std::string>(rend.at("name"));
            }
            if (rend.contains("color")) {
                const auto& color = rend.at("color");
                if (color.is_array() && color.as_array().size() >= 3) {
                    const auto& color_array = color.as_array();
                    renderable.color = ftxui::Color::RGB(
                        boost::json::value_to<int>(color_array[0]),
                        boost::json::value_to<int>(color_array[1]),
                        boost::json::value_to<int>(color_array[2])
                    );
                }
            }
        }
    }

    // Parse health
    if (json.is_object() && json.as_object().contains("health")) {
        const auto& health_obj = json.as_object().at("health");
        auto& health = entity->addComponent<HealthComponent>();
        if (health_obj.is_object() && health_obj.as_object().contains("max")) {
            health.max_hp = boost::json::value_to<int>(health_obj.as_object().at("max"));
        }
        if (health_obj.is_object() && health_obj.as_object().contains("current")) {
            health.hp = boost::json::value_to<int>(health_obj.as_object().at("current"));
        }
    }

    // Parse tags
    if (json.is_object() && json.as_object().contains("tags")) {
        const auto& tags_array = json.as_object().at("tags");
        if (tags_array.is_array()) {
            for (const auto& tag : tags_array.as_array()) {
                entity->addTag(boost::json::value_to<std::string>(tag));
            }
        }
    }

    return entity;
}

std::unique_ptr<Entity> EntityFactory::createRandomMonster(int x, int y, int dungeon_level) {
    // Monster tables by level
    static const struct {
        int min_level;
        std::vector<std::string> monsters;
    } monster_table[] = {
        {1, {"rat", "bat", "snake"}},
        {3, {"goblin", "skeleton", "zombie"}},
        {5, {"orc", "troll", "wraith"}},
        {7, {"ogre", "vampire", "demon"}},
        {10, {"dragon", "lich", "balrog"}}
    };

    // Find appropriate monsters for level
    std::vector<std::string> available;
    for (const auto& entry : monster_table) {
        if (dungeon_level >= entry.min_level) {
            available.insert(available.end(), entry.monsters.begin(), entry.monsters.end());
        }
    }

    if (available.empty()) {
        available.push_back("rat");  // Fallback
    }

    // Pick random monster
    static std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> dist(0, static_cast<int>(available.size()) - 1);
    std::string monster_id = available[dist(rng)];

    return createMonster(monster_id, x, y, dungeon_level);
}

std::unique_ptr<Entity> EntityFactory::createRandomItem(int x, int y, int dungeon_level,
                                                        const std::string& item_category) {
    // Item tables by level
    static const struct {
        int min_level;
        std::vector<std::string> items;
    } item_table[] = {
        {1, {"potion_minor", "scroll_identify", "dagger"}},
        {3, {"potion_health", "scroll_teleport", "sword", "leather_armor"}},
        {5, {"potion_major", "scroll_fireball", "longsword", "chainmail"}},
        {7, {"potion_full", "scroll_lightning", "greatsword", "plate_armor"}},
        {10, {"potion_legendary", "scroll_meteor", "excalibur", "dragon_scale"}}
    };

    std::vector<std::string> available;
    for (const auto& entry : item_table) {
        if (dungeon_level >= entry.min_level) {
            for (const auto& item : entry.items) {
                // Filter by category if specified
                if (item_category.empty() ||
                    (item_category == "potion" && item.find("potion") != std::string::npos) ||
                    (item_category == "scroll" && item.find("scroll") != std::string::npos) ||
                    (item_category == "weapon" && (item.find("sword") != std::string::npos ||
                                                   item.find("dagger") != std::string::npos)) ||
                    (item_category == "armor" && item.find("armor") != std::string::npos)) {
                    available.push_back(item);
                }
            }
        }
    }

    if (available.empty()) {
        available.push_back("potion_minor");  // Fallback
    }

    // Pick random item
    static std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> dist(0, static_cast<int>(available.size()) - 1);
    std::string item_id = available[dist(rng)];

    return createItem(item_id, x, y);
}

void EntityFactory::applyMonsterTemplate(Entity* entity, const std::string& monster_id, int level) {
    // Get template from DataLoader
    const auto* template_data = DataLoader::getInstance().getMonsterTemplate(monster_id);

    if (!template_data) {
        // Fallback to default monster if template not found
        auto& renderable = entity->addComponent<RenderableComponent>();
        renderable.glyph = 'm';
        renderable.name = "Unknown Monster";
        renderable.color = {255, 0, 255};

        auto& health = entity->addComponent<HealthComponent>();
        health.max_hp = 10;
        health.hp = 10;

        auto& combat = entity->addComponent<CombatComponent>();
        combat.min_damage = 1;
        combat.max_damage = 3;

        auto& ai = entity->addComponent<AIComponent>();
        ai.behavior = AIBehavior::AGGRESSIVE;

        return;
    }

    // Apply template data
    auto& renderable = entity->addComponent<RenderableComponent>();
    renderable.glyph = template_data->glyph;
    renderable.name = template_data->name;
    renderable.color = template_data->color;

    auto& health = entity->addComponent<HealthComponent>();
    health.max_hp = template_data->hp;
    health.hp = template_data->hp;

    auto& combat = entity->addComponent<CombatComponent>();
    combat.attack_bonus = template_data->attack;
    combat.defense_bonus = template_data->defense;

    auto& ai = entity->addComponent<AIComponent>();
    ai.behavior = template_data->aggressive ? AIBehavior::AGGRESSIVE : AIBehavior::WANDERING;

    // Add stats if needed
    auto& stats = entity->addComponent<StatsComponent>();
    stats.recalculateDerived();

    // Add experience value
    entity->addTag("xp:" + std::to_string(template_data->xp_value));

    // Apply combat bonuses from stats
    combat.attack_bonus = stats.accuracy_bonus;
    combat.damage_modifier = stats.damage_bonus;
    combat.defense_bonus = stats.armor_class;

    // Add loot component
    auto& loot = entity->addComponent<LootComponent>();
    loot.guaranteed_gold = level * 5;
    loot.random_gold_max = level * 10;

    // Add experience component
    if (level > 0) {
        auto& exp = entity->addComponent<ExperienceComponent>();
        exp.level = level;
        scaleMonsterToLevel(entity, level);
    }
}

void EntityFactory::applyItemTemplate(Entity* entity, const std::string& item_id, int quantity) {
    // Get template from DataLoader
    const auto* template_data = DataLoader::getInstance().getItemTemplate(item_id);

    if (!template_data) {
        // Fallback to default item if template not found
        auto& renderable = entity->addComponent<RenderableComponent>();
        renderable.glyph = '*';
        renderable.name = "Unknown Item";
        renderable.color = {255, 255, 255};

        auto& item = entity->addComponent<ItemComponent>();
        item.name = item_id;
        item.stack_size = quantity;
        item.max_stack = 1;
        item.value = 1;

        return;
    }

    // Apply template data
    auto& renderable = entity->addComponent<RenderableComponent>();
    renderable.glyph = template_data->symbol;
    renderable.name = template_data->name;
    renderable.color = template_data->color;

    auto& item = entity->addComponent<ItemComponent>();
    item.name = template_data->name;
    item.stack_size = quantity;
    item.max_stack = template_data->stackable ? template_data->max_stack : 1;
    item.value = template_data->value;
    item.weight = static_cast<float>(template_data->weight);

    // Set item type based on type string
    if (template_data->type == "potion") {
        item.item_type = ItemType::POTION;
        item.consumable = true;
        item.heal_amount = template_data->heal_amount;
    } else if (template_data->type == "scroll") {
        item.item_type = ItemType::SCROLL;
        item.consumable = true;
        item.damage_amount = template_data->damage_amount;
    } else if (template_data->type == "weapon") {
        item.item_type = ItemType::WEAPON;
        item.equippable = true;
        item.min_damage = template_data->min_damage;
        item.max_damage = template_data->max_damage;
        item.attack_bonus = template_data->attack_bonus;
        item.damage_bonus = template_data->attack_bonus;  // Using attack_bonus as damage bonus
    } else if (template_data->type == "armor") {
        item.item_type = ItemType::ARMOR;
        item.equippable = true;
        item.defense_bonus = template_data->defense_bonus;
    } else if (template_data->type == "shield") {
        item.item_type = ItemType::SHIELD;
        item.equippable = true;
        item.defense_bonus = template_data->defense_bonus;
    } else if (template_data->type == "ring") {
        item.item_type = ItemType::RING;
        item.equippable = true;
    } else if (template_data->type == "food") {
        item.item_type = ItemType::FOOD;
        item.consumable = true;
        item.heal_amount = template_data->heal_amount;
    } else if (template_data->type == "currency") {
        item.item_type = ItemType::MISC;
        item.max_stack = 99999;  // Large stack for currency
    } else {
        item.item_type = ItemType::MISC;
    }
}

void EntityFactory::scaleMonsterToLevel(Entity* entity, int level) {
    if (!entity || level <= 1) return;

    // Scale health
    auto* health = entity->getComponent<HealthComponent>();
    if (health) {
        health->max_hp = health->max_hp + (level - 1) * 5;
        health->hp = health->max_hp;
    }

    // Scale combat
    auto* combat = entity->getComponent<CombatComponent>();
    if (combat) {
        combat->min_damage += (level - 1) / 2;
        combat->max_damage += level - 1;
        combat->attack_bonus += (level - 1) / 3;
        combat->defense_bonus += (level - 1) / 4;
    }

    // Scale stats
    auto* stats = entity->getComponent<StatsComponent>();
    if (stats) {
        stats->strength += (level - 1) / 2;
        stats->constitution += (level - 1) / 2;
        stats->recalculateDerived();
    }
}

void EntityFactory::generateRandomProperties(Entity* entity, int quality) {
    auto* item = entity->getComponent<ItemComponent>();
    if (!item) return;

    static std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> bonus_dist(1, quality);

    // Add random bonuses based on quality
    if (item->item_type == ItemType::WEAPON) {
        item->attack_bonus += bonus_dist(rng);
        item->damage_bonus += bonus_dist(rng);

        // Chance for special property
        if (quality >= 3 && (rng() % 3 == 0)) {
            entity->addTag("flaming");  // Fire damage
            item->name = "Flaming " + item->name;
        }
    } else if (item->item_type == ItemType::ARMOR) {
        item->defense_bonus += bonus_dist(rng);

        // Chance for resistance
        if (quality >= 3 && (rng() % 3 == 0)) {
            entity->addTag("fire_resistant");
            item->name = "Fire-Resistant " + item->name;
        }
    }

    // Update value
    item->value = item->value * (1 + quality);
}

} // namespace ecs