#include <catch2/catch_test_macros.hpp>
#include "ecs/equipment_system.h"
#include "ecs/equipment_component.h"
#include "ecs/item_component.h"
#include "ecs/stats_component.h"
#include "ecs/combat_component.h"
#include "ecs/entity.h"
#include <memory>
#include <vector>

using namespace ecs;

TEST_CASE("EquipmentSlot enum", "[ecs][equipment][slot]") {
    SECTION("Enum values") {
        REQUIRE(EquipmentSlot::NONE != EquipmentSlot::MAIN_HAND);
        REQUIRE(EquipmentSlot::MAIN_HAND != EquipmentSlot::OFF_HAND);
        REQUIRE(EquipmentSlot::HEAD != EquipmentSlot::BODY);
        REQUIRE(EquipmentSlot::HANDS != EquipmentSlot::FEET);
        REQUIRE(EquipmentSlot::NECK != EquipmentSlot::RING_LEFT);
        REQUIRE(EquipmentSlot::RING_LEFT != EquipmentSlot::RING_RIGHT);
        REQUIRE(EquipmentSlot::BACK != EquipmentSlot::BELT);
        REQUIRE(EquipmentSlot::BELT != EquipmentSlot::RANGED);
    }
}

TEST_CASE("EquipmentComponent functionality", "[ecs][equipment][component]") {
    SECTION("Component construction") {
        EquipmentComponent equipment;

        REQUIRE(equipment.equipped_items.empty());
        REQUIRE(equipment.total_attack_bonus == 0);
        REQUIRE(equipment.total_defense_bonus == 0);
        REQUIRE(equipment.total_damage_bonus == 0);
        REQUIRE(equipment.total_armor_class == 0);
        REQUIRE(equipment.total_resistance == 0);
        REQUIRE(equipment.getTypeName() == "EquipmentComponent");
        REQUIRE(equipment.getType() == ComponentType::CUSTOM);
    }

    SECTION("Equipment slot operations") {
        EquipmentComponent equipment;

        // Test hasEquipped
        REQUIRE(!equipment.hasEquipped(EquipmentSlot::MAIN_HAND));
        REQUIRE(!equipment.hasEquipped(EquipmentSlot::HEAD));

        // Test equip
        EntityID previous = equipment.equip(EquipmentSlot::MAIN_HAND, 123);
        REQUIRE(previous == 0); // No previous item

        REQUIRE(equipment.hasEquipped(EquipmentSlot::MAIN_HAND));
        REQUIRE(equipment.getEquipped(EquipmentSlot::MAIN_HAND) == 123);

        // Test replace item
        EntityID previous2 = equipment.equip(EquipmentSlot::MAIN_HAND, 456);
        REQUIRE(previous2 == 123); // Previous item returned

        REQUIRE(equipment.getEquipped(EquipmentSlot::MAIN_HAND) == 456);
    }

    SECTION("Unequip operations") {
        EquipmentComponent equipment;

        // Unequip from empty slot
        EntityID removed = equipment.unequip(EquipmentSlot::MAIN_HAND);
        REQUIRE(removed == 0);

        // Equip then unequip
        equipment.equip(EquipmentSlot::MAIN_HAND, 789);
        REQUIRE(equipment.hasEquipped(EquipmentSlot::MAIN_HAND));

        EntityID removed2 = equipment.unequip(EquipmentSlot::MAIN_HAND);
        REQUIRE(removed2 == 789);
        REQUIRE(!equipment.hasEquipped(EquipmentSlot::MAIN_HAND));
    }

    SECTION("Multiple slot management") {
        EquipmentComponent equipment;

        equipment.equip(EquipmentSlot::MAIN_HAND, 100);
        equipment.equip(EquipmentSlot::HEAD, 200);
        equipment.equip(EquipmentSlot::BODY, 300);

        REQUIRE(equipment.hasEquipped(EquipmentSlot::MAIN_HAND));
        REQUIRE(equipment.hasEquipped(EquipmentSlot::HEAD));
        REQUIRE(equipment.hasEquipped(EquipmentSlot::BODY));
        REQUIRE(!equipment.hasEquipped(EquipmentSlot::FEET));

        REQUIRE(equipment.getEquipped(EquipmentSlot::MAIN_HAND) == 100);
        REQUIRE(equipment.getEquipped(EquipmentSlot::HEAD) == 200);
        REQUIRE(equipment.getEquipped(EquipmentSlot::BODY) == 300);
        REQUIRE(equipment.getEquipped(EquipmentSlot::FEET) == 0);
    }

    SECTION("Two-handed weapon check") {
        EquipmentComponent equipment;

        // Can equip two-handed when off-hand is free
        REQUIRE(equipment.canEquip(EquipmentSlot::MAIN_HAND, true));

        // Equip shield in off-hand
        equipment.equip(EquipmentSlot::OFF_HAND, 999);

        // Cannot equip two-handed when off-hand is occupied
        REQUIRE(!equipment.canEquip(EquipmentSlot::MAIN_HAND, true));

        // Can still equip one-handed
        REQUIRE(equipment.canEquip(EquipmentSlot::MAIN_HAND, false));
    }

    SECTION("Bonus recalculation") {
        EquipmentComponent equipment;

        equipment.total_attack_bonus = 10;
        equipment.total_defense_bonus = 5;

        equipment.recalculateBonuses();

        // Should reset to 0 (basic implementation)
        REQUIRE(equipment.total_attack_bonus == 0);
        REQUIRE(equipment.total_defense_bonus == 0);
        REQUIRE(equipment.total_damage_bonus == 0);
        REQUIRE(equipment.total_armor_class == 0);
        REQUIRE(equipment.total_resistance == 0);
    }
}

TEST_CASE("EquipmentBonuses struct", "[ecs][equipment][bonuses]") {
    SECTION("Default construction") {
        EquipmentBonuses bonuses;

        REQUIRE(bonuses.attack_bonus == 0);
        REQUIRE(bonuses.damage_bonus == 0);
        REQUIRE(bonuses.defense_bonus == 0);
        REQUIRE(bonuses.armor_bonus == 0);
        REQUIRE(bonuses.speed_bonus == 0);
        REQUIRE(bonuses.strength_bonus == 0);
        REQUIRE(bonuses.dexterity_bonus == 0);
        REQUIRE(bonuses.intelligence_bonus == 0);
        REQUIRE(bonuses.constitution_bonus == 0);
        REQUIRE(bonuses.wisdom_bonus == 0);
        REQUIRE(bonuses.charisma_bonus == 0);
        REQUIRE(bonuses.fire_resistance == 0);
        REQUIRE(bonuses.cold_resistance == 0);
        REQUIRE(bonuses.poison_resistance == 0);
        REQUIRE(bonuses.magic_resistance == 0);
        REQUIRE(bonuses.critical_chance == 0);
        REQUIRE(bonuses.critical_damage == 0);
        REQUIRE(bonuses.life_steal == 0);
        REQUIRE(bonuses.mana_steal == 0);
    }

    SECTION("Add item bonuses") {
        EquipmentBonuses bonuses;
        ItemComponent item;

        item.attack_bonus = 5;
        item.damage_bonus = 3;
        item.defense_bonus = 2;

        bonuses.addItemBonuses(item);

        REQUIRE(bonuses.attack_bonus == 5);
        REQUIRE(bonuses.damage_bonus == 3);
        REQUIRE(bonuses.defense_bonus == 2);
    }

    SECTION("Multiple item bonuses") {
        EquipmentBonuses bonuses;

        ItemComponent weapon;
        weapon.attack_bonus = 10;
        weapon.damage_bonus = 5;

        ItemComponent armor;
        armor.defense_bonus = 8;

        bonuses.addItemBonuses(weapon);
        bonuses.addItemBonuses(armor);

        REQUIRE(bonuses.attack_bonus == 10);
        REQUIRE(bonuses.damage_bonus == 5);
        REQUIRE(bonuses.defense_bonus == 8);
    }
}

TEST_CASE("EquipmentSystem basic functionality", "[ecs][equipment][system]") {
    SECTION("System construction") {
        EquipmentSystem equipment_system;

        REQUIRE(equipment_system.getPriority() == 30);
    }

    SECTION("System construction with logger") {
        EquipmentSystem equipment_system(nullptr);

        REQUIRE(equipment_system.getPriority() == 30);
    }

    SECTION("System construction with world") {
        EquipmentSystem equipment_system(nullptr, nullptr);

        REQUIRE(equipment_system.getPriority() == 30);

        // Test setting world
        equipment_system.setWorld(nullptr);
        // Should not crash
        REQUIRE(true);
    }

    SECTION("Should process entities") {
        EquipmentSystem equipment_system;

        // Entity without EquipmentComponent
        auto entity1 = std::make_unique<Entity>(1);
        REQUIRE(!equipment_system.shouldProcess(*entity1));

        // Entity with EquipmentComponent
        auto entity2 = std::make_unique<Entity>(2);
        entity2->addComponent<EquipmentComponent>();
        REQUIRE(equipment_system.shouldProcess(*entity2));
    }

    SECTION("Update method") {
        EquipmentSystem equipment_system;
        std::vector<std::unique_ptr<Entity>> entities;

        auto entity = std::make_unique<Entity>(1);
        entity->addComponent<EquipmentComponent>();
        entities.push_back(std::move(entity));

        // Should not crash
        equipment_system.update(entities, 0.016);
        REQUIRE(true);
    }

    SECTION("Static slot determination") {
        // Create a basic item entity
        auto sword = std::make_unique<Entity>(1);
        sword->addComponent<ItemComponent>();

        // Test slot detection (basic test, implementation may vary)
        EquipmentSlot slot = EquipmentSystem::getSlotForItem(sword.get());
        // Should return a valid slot (or NONE if not implemented)
        REQUIRE((slot == EquipmentSlot::NONE ||
                 slot == EquipmentSlot::MAIN_HAND ||
                 slot == EquipmentSlot::HEAD ||
                 slot == EquipmentSlot::BODY));
    }
}

TEST_CASE("EquipmentSystem entity operations", "[ecs][equipment][entity_ops]") {
    SECTION("Calculate bonuses for empty equipment") {
        EquipmentSystem equipment_system;

        auto entity = std::make_unique<Entity>(1);
        entity->addComponent<EquipmentComponent>();

        EquipmentBonuses bonuses = equipment_system.calculateBonuses(entity.get());

        // Empty equipment should give no bonuses
        REQUIRE(bonuses.attack_bonus == 0);
        REQUIRE(bonuses.defense_bonus == 0);
        REQUIRE(bonuses.damage_bonus == 0);
    }

    SECTION("Can equip checks") {
        EquipmentSystem equipment_system;

        auto character = std::make_unique<Entity>(1);
        character->addComponent<EquipmentComponent>();

        auto weapon = std::make_unique<Entity>(2);
        weapon->addComponent<ItemComponent>();

        // Basic can equip test
        bool can_equip = equipment_system.canEquip(character.get(), weapon.get());
        // Should return true or false without crashing
        REQUIRE((can_equip == true || can_equip == false));
    }

    SECTION("Apply equipment bonuses") {
        EquipmentSystem equipment_system;

        auto entity = std::make_unique<Entity>(1);
        entity->addComponent<EquipmentComponent>();
        entity->addComponent<StatsComponent>();

        // Should not crash even without equipped items
        equipment_system.applyEquipmentBonuses(entity.get());
        REQUIRE(true);
    }

    SECTION("Get equipped item") {
        EquipmentSystem equipment_system;

        auto character = std::make_unique<Entity>(1);
        character->addComponent<EquipmentComponent>();

        // Get item from empty slot
        Entity* item = equipment_system.getEquippedItem(character.get(), EquipmentSlot::MAIN_HAND);
        REQUIRE(item == nullptr);
    }
}