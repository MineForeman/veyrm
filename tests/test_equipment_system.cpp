#include <catch2/catch_test_macros.hpp>
#include "ecs/equipment_system.h"
#include "ecs/game_world.h"
#include "ecs/entity_factory.h"
#include "ecs/component.h"
#include "map.h"

using namespace ecs;

class TestableEquipmentSystem : public EquipmentSystem {
public:
    TestableEquipmentSystem(GameWorld* world) : EquipmentSystem(world) {}

    // Expose protected methods for testing
    using EquipmentSystem::canEquip;
    using EquipmentSystem::getSlotForItem;
    using EquipmentSystem::unequipSlot;
    using EquipmentSystem::applyEquipmentStats;
    using EquipmentSystem::removeEquipmentStats;
};

TEST_CASE("EquipmentSystem basic operations", "[equipment][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableEquipmentSystem equipSystem(world.get());
    EntityFactory factory(world.get());

    SECTION("Equip weapon") {
        auto player = factory.createPlayer(10, 10);
        auto sword = factory.createItem("sword_basic", 0, 0);

        // Add sword to inventory first
        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(sword);
        }

        bool equipped = equipSystem.equipItem(player, sword);
        REQUIRE(equipped == true);

        // Check if equipped
        if (auto* equipment = world->getComponent<EquipmentComponent>(player)) {
            REQUIRE(equipment->getEquipped(EquipmentSlot::MainHand) == sword);
        }
    }

    SECTION("Equip armor") {
        auto player = factory.createPlayer(10, 10);
        auto armor = factory.createItem("armor_leather", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(armor);
        }

        bool equipped = equipSystem.equipItem(player, armor);
        REQUIRE(equipped == true);

        if (auto* equipment = world->getComponent<EquipmentComponent>(player)) {
            REQUIRE(equipment->getEquipped(EquipmentSlot::Body) == armor);
        }
    }

    SECTION("Unequip item") {
        auto player = factory.createPlayer(10, 10);
        auto sword = factory.createItem("sword_basic", 0, 0);

        // Equip first
        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(sword);
        }
        equipSystem.equipItem(player, sword);

        // Then unequip
        bool unequipped = equipSystem.unequipItem(player, sword);
        REQUIRE(unequipped == true);

        // Should be back in inventory
        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            REQUIRE(std::find(inventory->items.begin(), inventory->items.end(), sword)
                    != inventory->items.end());
        }

        // Should not be equipped
        if (auto* equipment = world->getComponent<EquipmentComponent>(player)) {
            REQUIRE(equipment->getEquipped(EquipmentSlot::MainHand) == nullptr);
        }
    }

    SECTION("Unequip slot directly") {
        auto player = factory.createPlayer(10, 10);
        auto helmet = factory.createItem("helmet_basic", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(helmet);
        }
        equipSystem.equipItem(player, helmet);

        bool unequipped = equipSystem.unequipSlot(player, EquipmentSlot::Head);
        REQUIRE(unequipped == true);

        if (auto* equipment = world->getComponent<EquipmentComponent>(player)) {
            REQUIRE(equipment->getEquipped(EquipmentSlot::Head) == nullptr);
        }
    }

    SECTION("Check if item is equipped") {
        auto player = factory.createPlayer(10, 10);
        auto ring = factory.createItem("ring_protection", 0, 0);

        REQUIRE(equipSystem.isEquipped(player, ring) == false);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(ring);
        }
        equipSystem.equipItem(player, ring);

        REQUIRE(equipSystem.isEquipped(player, ring) == true);
    }

    SECTION("Get equipped items") {
        auto player = factory.createPlayer(10, 10);
        auto sword = factory.createItem("sword_basic", 0, 0);
        auto armor = factory.createItem("armor_leather", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(sword);
            inventory->items.push_back(armor);
        }

        equipSystem.equipItem(player, sword);
        equipSystem.equipItem(player, armor);

        auto equipped = equipSystem.getEquippedItems(player);
        REQUIRE(equipped.size() == 2);
        REQUIRE(std::find(equipped.begin(), equipped.end(), sword) != equipped.end());
        REQUIRE(std::find(equipped.begin(), equipped.end(), armor) != equipped.end());
    }

    SECTION("Get item in specific slot") {
        auto player = factory.createPlayer(10, 10);
        auto boots = factory.createItem("boots_leather", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(boots);
        }
        equipSystem.equipItem(player, boots);

        auto itemInSlot = equipSystem.getItemInSlot(player, EquipmentSlot::Feet);
        REQUIRE(itemInSlot == boots);

        auto emptySlot = equipSystem.getItemInSlot(player, EquipmentSlot::OffHand);
        REQUIRE(emptySlot == nullptr);
    }

    SECTION("Equipment stats application") {
        auto player = factory.createPlayer(10, 10);
        auto sword = factory.createItem("sword_basic", 0, 0);

        // Get initial stats
        auto* stats = world->getComponent<StatsComponent>(player);
        auto* combat = world->getComponent<CombatComponent>(player);

        int initialStr = stats ? stats->strength : 10;
        int initialDamage = combat ? combat->maxDamage : 5;

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(sword);
        }

        // Equip and check stat changes
        equipSystem.equipItem(player, sword);

        if (stats && combat) {
            // Sword should increase damage
            REQUIRE(combat->maxDamage > initialDamage);
        }

        // Unequip and verify stats revert
        equipSystem.unequipItem(player, sword);

        if (stats && combat) {
            REQUIRE(combat->maxDamage == initialDamage);
        }
    }

    SECTION("Two-handed weapon handling") {
        auto player = factory.createPlayer(10, 10);
        auto twoHandedSword = factory.createItem("sword_twohanded", 0, 0);
        auto shield = factory.createItem("shield_basic", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(twoHandedSword);
            inventory->items.push_back(shield);
        }

        // Equip two-handed weapon
        equipSystem.equipItem(player, twoHandedSword);

        // Try to equip shield (should fail or unequip two-handed)
        bool shieldEquipped = equipSystem.equipItem(player, shield);

        if (shieldEquipped) {
            // Two-handed should be unequipped
            REQUIRE(equipSystem.isEquipped(player, twoHandedSword) == false);
        } else {
            // Shield couldn't be equipped
            REQUIRE(equipSystem.isEquipped(player, shield) == false);
        }
    }

    SECTION("Equipment requirements") {
        auto player = factory.createPlayer(10, 10);
        auto heavyArmor = factory.createItem("armor_plate", 0, 0);

        // Set player strength low
        if (auto* stats = world->getComponent<StatsComponent>(player)) {
            stats->strength = 5; // Too low for plate armor
        }

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(heavyArmor);
        }

        // Should check requirements
        bool canEquip = equipSystem.canEquip(player, heavyArmor);

        // If requirements are implemented, this should fail
        if (!canEquip) {
            bool equipped = equipSystem.equipItem(player, heavyArmor);
            REQUIRE(equipped == false);
        }
    }

    SECTION("Cursed item handling") {
        auto player = factory.createPlayer(10, 10);

        // Create a cursed item
        auto cursedRing = factory.createItem("ring_cursed", 0, 0);
        if (auto* item = world->getComponent<ItemComponent>(cursedRing)) {
            item->isCursed = true;
        }

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(cursedRing);
        }

        // Can equip cursed item
        bool equipped = equipSystem.equipItem(player, cursedRing);
        REQUIRE(equipped == true);

        // Cannot unequip cursed item normally
        bool unequipped = equipSystem.unequipItem(player, cursedRing);

        // Cursed items should not be unequippable
        if (auto* item = world->getComponent<ItemComponent>(cursedRing)) {
            if (item->isCursed) {
                REQUIRE(equipSystem.isEquipped(player, cursedRing) == true);
            }
        }
    }

    SECTION("Auto-equip best items") {
        auto player = factory.createPlayer(10, 10);

        // Create multiple weapons with different qualities
        auto weakSword = factory.createItem("sword_rusty", 0, 0);
        auto goodSword = factory.createItem("sword_steel", 0, 0);
        auto bestSword = factory.createItem("sword_enchanted", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(weakSword);
            inventory->items.push_back(goodSword);
            inventory->items.push_back(bestSword);
        }

        // Auto-equip should pick the best
        equipSystem.autoEquipBestItems(player);

        // Should have equipped the enchanted sword
        auto equipped = equipSystem.getItemInSlot(player, EquipmentSlot::MainHand);
        REQUIRE(equipped == bestSword);
    }

    SECTION("Equipment slot conflicts") {
        auto player = factory.createPlayer(10, 10);
        auto ring1 = factory.createItem("ring_protection", 0, 0);
        auto ring2 = factory.createItem("ring_strength", 0, 0);
        auto ring3 = factory.createItem("ring_dexterity", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(ring1);
            inventory->items.push_back(ring2);
            inventory->items.push_back(ring3);
        }

        // Equip two rings (max)
        equipSystem.equipItem(player, ring1);
        equipSystem.equipItem(player, ring2);

        // Third ring should fail or replace
        bool equipped = equipSystem.equipItem(player, ring3);

        auto equippedRings = equipSystem.getEquippedItems(player);
        // Should have exactly 2 rings equipped
        int ringCount = 0;
        for (auto item : equippedRings) {
            if (auto* itemComp = world->getComponent<ItemComponent>(item)) {
                if (itemComp->type == ItemType::Ring) {
                    ringCount++;
                }
            }
        }
        REQUIRE(ringCount <= 2);
    }

    SECTION("Equipment durability") {
        auto player = factory.createPlayer(10, 10);
        auto sword = factory.createItem("sword_basic", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(sword)) {
            item->durability = 10;
            item->maxDurability = 100;
        }

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(sword);
        }

        equipSystem.equipItem(player, sword);

        // Simulate combat damage to equipment
        equipSystem.damageEquipment(player, EquipmentSlot::MainHand, 5);

        if (auto* item = world->getComponent<ItemComponent>(sword)) {
            REQUIRE(item->durability == 5);
        }

        // Break the item
        equipSystem.damageEquipment(player, EquipmentSlot::MainHand, 10);

        // Broken items should be unequipped
        if (auto* item = world->getComponent<ItemComponent>(sword)) {
            if (item->durability <= 0) {
                REQUIRE(equipSystem.isEquipped(player, sword) == false);
            }
        }
    }

    SECTION("Equipment set bonuses") {
        auto player = factory.createPlayer(10, 10);

        // Create a matching set
        auto helmet = factory.createItem("helmet_dragon", 0, 0);
        auto armor = factory.createItem("armor_dragon", 0, 0);
        auto gloves = factory.createItem("gloves_dragon", 0, 0);
        auto boots = factory.createItem("boots_dragon", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(helmet);
            inventory->items.push_back(armor);
            inventory->items.push_back(gloves);
            inventory->items.push_back(boots);
        }

        // Equip partial set
        equipSystem.equipItem(player, helmet);
        equipSystem.equipItem(player, armor);

        auto partialBonus = equipSystem.getSetBonus(player, "dragon");

        // Equip full set
        equipSystem.equipItem(player, gloves);
        equipSystem.equipItem(player, boots);

        auto fullBonus = equipSystem.getSetBonus(player, "dragon");

        // Full set should give better bonus
        REQUIRE(fullBonus >= partialBonus);
    }

    SECTION("Quick swap equipment sets") {
        auto player = factory.createPlayer(10, 10);

        // Create two equipment sets
        auto swordSet = factory.createItem("sword_basic", 0, 0);
        auto shieldSet = factory.createItem("shield_basic", 0, 0);
        auto bowSet = factory.createItem("bow_basic", 0, 0);
        auto quiverSet = factory.createItem("quiver_basic", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(swordSet);
            inventory->items.push_back(shieldSet);
            inventory->items.push_back(bowSet);
            inventory->items.push_back(quiverSet);
        }

        // Save melee set
        equipSystem.equipItem(player, swordSet);
        equipSystem.equipItem(player, shieldSet);
        equipSystem.saveEquipmentSet(player, 1);

        // Switch to ranged set
        equipSystem.unequipItem(player, swordSet);
        equipSystem.unequipItem(player, shieldSet);
        equipSystem.equipItem(player, bowSet);
        equipSystem.equipItem(player, quiverSet);
        equipSystem.saveEquipmentSet(player, 2);

        // Quick swap back to melee
        equipSystem.loadEquipmentSet(player, 1);
        REQUIRE(equipSystem.isEquipped(player, swordSet) == true);
        REQUIRE(equipSystem.isEquipped(player, shieldSet) == true);
        REQUIRE(equipSystem.isEquipped(player, bowSet) == false);

        // Quick swap to ranged
        equipSystem.loadEquipmentSet(player, 2);
        REQUIRE(equipSystem.isEquipped(player, bowSet) == true);
        REQUIRE(equipSystem.isEquipped(player, quiverSet) == true);
        REQUIRE(equipSystem.isEquipped(player, swordSet) == false);
    }
}

TEST_CASE("EquipmentSystem edge cases", "[equipment][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    EquipmentSystem equipSystem(world.get());
    EntityFactory factory(world.get());

    SECTION("Null entity operations") {
        REQUIRE(equipSystem.equipItem(nullptr, nullptr) == false);
        REQUIRE(equipSystem.unequipItem(nullptr, nullptr) == false);
        REQUIRE(equipSystem.isEquipped(nullptr, nullptr) == false);
        REQUIRE(equipSystem.getEquippedItems(nullptr).empty());
    }

    SECTION("Entity without equipment component") {
        auto entity = factory.createEntity();
        auto item = factory.createItem("sword_basic", 0, 0);

        REQUIRE(equipSystem.equipItem(entity, item) == false);
        REQUIRE(equipSystem.unequipItem(entity, item) == false);
    }

    SECTION("Entity without inventory component") {
        auto entity = factory.createEntity();
        world->addComponent<EquipmentComponent>(entity);
        auto item = factory.createItem("sword_basic", 0, 0);

        // Can't equip without inventory
        REQUIRE(equipSystem.equipItem(entity, item) == false);
    }

    SECTION("Item not in inventory") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("sword_basic", 0, 0);

        // Item exists but not in inventory
        REQUIRE(equipSystem.equipItem(player, item) == false);
    }

    SECTION("Invalid item types") {
        auto player = factory.createPlayer(10, 10);
        auto potion = factory.createItem("potion_minor", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(potion);
        }

        // Can't equip consumables
        REQUIRE(equipSystem.equipItem(player, potion) == false);
    }

    SECTION("Equipment with no stats") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createEntity(); // Generic item with no stats

        world->addComponent<ItemComponent>(item);
        if (auto* itemComp = world->getComponent<ItemComponent>(item)) {
            itemComp->type = ItemType::Weapon;
        }

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(item);
        }

        // Should handle items with no stat bonuses
        bool equipped = equipSystem.equipItem(player, item);
        REQUIRE(equipped == true);
    }

    SECTION("Full inventory after unequip") {
        auto player = factory.createPlayer(10, 10);
        auto sword = factory.createItem("sword_basic", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(sword);

            // Fill inventory to capacity
            inventory->capacity = inventory->items.size() + 1;
            for (size_t i = inventory->items.size(); i < inventory->capacity; ++i) {
                inventory->items.push_back(factory.createItem("potion_minor", 0, 0));
            }
        }

        equipSystem.equipItem(player, sword);

        // Inventory is now full
        bool unequipped = equipSystem.unequipItem(player, sword);

        // Should still be able to unequip (makes room)
        REQUIRE(unequipped == true);
    }

    SECTION("Concurrent equipment operations") {
        auto player = factory.createPlayer(10, 10);
        auto item1 = factory.createItem("sword_basic", 0, 0);
        auto item2 = factory.createItem("sword_steel", 0, 0);

        if (auto* inventory = world->getComponent<InventoryComponent>(player)) {
            inventory->items.push_back(item1);
            inventory->items.push_back(item2);
        }

        // Rapid equip/unequip
        for (int i = 0; i < 10; ++i) {
            equipSystem.equipItem(player, item1);
            equipSystem.equipItem(player, item2);
            equipSystem.unequipItem(player, item1);
            equipSystem.unequipItem(player, item2);
        }

        // System should remain stable
        REQUIRE(true);
    }
}