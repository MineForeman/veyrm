#include <catch2/catch_test_macros.hpp>
#include "ecs/inventory_system.h"
#include "ecs/game_world.h"
#include "ecs/entity_factory.h"
#include "ecs/component.h"
#include "map.h"

using namespace ecs;

class TestableInventorySystem : public InventorySystem {
public:
    TestableInventorySystem(Map* map, ILogger* logger)
        : InventorySystem(map, logger) {}

    // Expose protected methods for testing
    using InventorySystem::canAddItem;
    using InventorySystem::findItemIndex;
    using InventorySystem::stackItems;
    using InventorySystem::sortInventory;
    using InventorySystem::calculateWeight;
};

TEST_CASE("InventorySystem basic operations", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableInventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Add item to inventory") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("potion_minor", 10, 10);

        bool added = invSystem.addItem(player, item);
        REQUIRE(added == true);

        auto* inventory = world->getComponent<InventoryComponent>(player);
        REQUIRE(inventory != nullptr);
        REQUIRE(inventory->items.size() == 1);
        REQUIRE(inventory->items[0] == item);
    }

    SECTION("Remove item from inventory") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("potion_minor", 10, 10);

        invSystem.addItem(player, item);
        bool removed = invSystem.removeItem(player, item);
        REQUIRE(removed == true);

        auto* inventory = world->getComponent<InventoryComponent>(player);
        REQUIRE(inventory->items.empty());
    }

    SECTION("Check if has item") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("sword_basic", 10, 10);

        REQUIRE(invSystem.hasItem(player, item) == false);

        invSystem.addItem(player, item);
        REQUIRE(invSystem.hasItem(player, item) == true);
    }

    SECTION("Get inventory items") {
        auto player = factory.createPlayer(10, 10);
        auto item1 = factory.createItem("potion_minor", 0, 0);
        auto item2 = factory.createItem("sword_basic", 0, 0);

        invSystem.addItem(player, item1);
        invSystem.addItem(player, item2);

        auto items = invSystem.getItems(player);
        REQUIRE(items.size() == 2);
        REQUIRE(std::find(items.begin(), items.end(), item1) != items.end());
        REQUIRE(std::find(items.begin(), items.end(), item2) != items.end());
    }

    SECTION("Get item count") {
        auto player = factory.createPlayer(10, 10);

        REQUIRE(invSystem.getItemCount(player) == 0);

        auto item1 = factory.createItem("potion_minor", 0, 0);
        auto item2 = factory.createItem("potion_minor", 0, 0);

        invSystem.addItem(player, item1);
        invSystem.addItem(player, item2);

        REQUIRE(invSystem.getItemCount(player) == 2);
    }

    SECTION("Check inventory full") {
        auto player = factory.createPlayer(10, 10);
        auto* inventory = world->getComponent<InventoryComponent>(player);

        if (inventory) {
            inventory->capacity = 2;
        }

        REQUIRE(invSystem.isFull(player) == false);

        invSystem.addItem(player, factory.createItem("potion_minor", 0, 0));
        REQUIRE(invSystem.isFull(player) == false);

        invSystem.addItem(player, factory.createItem("potion_minor", 0, 0));
        REQUIRE(invSystem.isFull(player) == true);
    }

    SECTION("Get available space") {
        auto player = factory.createPlayer(10, 10);
        auto* inventory = world->getComponent<InventoryComponent>(player);

        if (inventory) {
            inventory->capacity = 10;
        }

        REQUIRE(invSystem.getAvailableSpace(player) == 10);

        invSystem.addItem(player, factory.createItem("potion_minor", 0, 0));
        invSystem.addItem(player, factory.createItem("sword_basic", 0, 0));

        REQUIRE(invSystem.getAvailableSpace(player) == 8);
    }

    SECTION("Transfer item between inventories") {
        auto player = factory.createPlayer(10, 10);
        auto chest = factory.createEntity();
        world->addComponent<InventoryComponent>(chest, 20);

        auto item = factory.createItem("gold_coins", 0, 0);
        invSystem.addItem(player, item);

        bool transferred = invSystem.transferItem(item, player, chest);
        REQUIRE(transferred == true);

        REQUIRE(invSystem.hasItem(player, item) == false);
        REQUIRE(invSystem.hasItem(chest, item) == true);
    }

    SECTION("Drop item") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("sword_basic", 0, 0);

        invSystem.addItem(player, item);
        bool dropped = invSystem.dropItem(player, item, 15, 15);
        REQUIRE(dropped == true);

        REQUIRE(invSystem.hasItem(player, item) == false);

        // Item should be at drop location
        if (auto* pos = world->getComponent<PositionComponent>(item)) {
            REQUIRE(pos->x == 15);
            REQUIRE(pos->y == 15);
        }
    }

    SECTION("Pick up item") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("potion_minor", 10, 10);

        bool picked = invSystem.pickupItem(player, item);
        REQUIRE(picked == true);

        REQUIRE(invSystem.hasItem(player, item) == true);

        // Item position should be invalid (in inventory)
        if (auto* pos = world->getComponent<PositionComponent>(item)) {
            REQUIRE(pos->x == -1);
            REQUIRE(pos->y == -1);
        }
    }

    SECTION("Use consumable item") {
        auto player = factory.createPlayer(10, 10);
        auto potion = factory.createItem("potion_minor", 0, 0);

        invSystem.addItem(player, potion);

        // Set player health low
        if (auto* health = world->getComponent<HealthComponent>(player)) {
            health->hp = 50;
            health->maxHp = 100;
        }

        bool used = invSystem.useItem(player, potion);
        REQUIRE(used == true);

        // Potion should be consumed
        REQUIRE(invSystem.hasItem(player, potion) == false);

        // Health should be increased
        if (auto* health = world->getComponent<HealthComponent>(player)) {
            REQUIRE(health->hp > 50);
        }
    }
}

TEST_CASE("InventorySystem stacking", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableInventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Stack identical items") {
        auto player = factory.createPlayer(10, 10);

        // Create stackable items
        auto coins1 = factory.createItem("gold_coins", 0, 0);
        auto coins2 = factory.createItem("gold_coins", 0, 0);

        if (auto* item1 = world->getComponent<ItemComponent>(coins1)) {
            item1->isStackable = true;
            item1->stackSize = 50;
        }
        if (auto* item2 = world->getComponent<ItemComponent>(coins2)) {
            item2->isStackable = true;
            item2->stackSize = 30;
        }

        invSystem.addItem(player, coins1);
        invSystem.addItem(player, coins2);

        // Should combine into one stack
        auto items = invSystem.getItems(player);
        REQUIRE(items.size() == 1);

        // Stack should have combined count
        if (auto* item = world->getComponent<ItemComponent>(items[0])) {
            REQUIRE(item->stackSize == 80);
        }
    }

    SECTION("Respect max stack size") {
        auto player = factory.createPlayer(10, 10);

        auto arrows1 = factory.createItem("arrow", 0, 0);
        auto arrows2 = factory.createItem("arrow", 0, 0);

        if (auto* item1 = world->getComponent<ItemComponent>(arrows1)) {
            item1->isStackable = true;
            item1->stackSize = 90;
            item1->maxStackSize = 99;
        }
        if (auto* item2 = world->getComponent<ItemComponent>(arrows2)) {
            item2->isStackable = true;
            item2->stackSize = 20;
            item2->maxStackSize = 99;
        }

        invSystem.addItem(player, arrows1);
        invSystem.addItem(player, arrows2);

        auto items = invSystem.getItems(player);
        REQUIRE(items.size() == 2); // Should split into two stacks

        // First stack should be at max
        if (auto* item = world->getComponent<ItemComponent>(items[0])) {
            REQUIRE(item->stackSize == 99);
        }
        // Second stack has remainder
        if (auto* item = world->getComponent<ItemComponent>(items[1])) {
            REQUIRE(item->stackSize == 11);
        }
    }

    SECTION("Split stack") {
        auto player = factory.createPlayer(10, 10);

        auto coins = factory.createItem("gold_coins", 0, 0);
        if (auto* item = world->getComponent<ItemComponent>(coins)) {
            item->isStackable = true;
            item->stackSize = 100;
        }

        invSystem.addItem(player, coins);

        auto splitItem = invSystem.splitStack(player, coins, 40);
        REQUIRE(splitItem != nullptr);

        // Original stack reduced
        if (auto* item = world->getComponent<ItemComponent>(coins)) {
            REQUIRE(item->stackSize == 60);
        }

        // New stack created
        if (auto* item = world->getComponent<ItemComponent>(splitItem)) {
            REQUIRE(item->stackSize == 40);
        }
    }
}

TEST_CASE("InventorySystem weight management", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableInventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Calculate total weight") {
        auto player = factory.createPlayer(10, 10);

        auto sword = factory.createItem("sword_basic", 0, 0);
        auto armor = factory.createItem("armor_leather", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(sword)) {
            item->weight = 3.5f;
        }
        if (auto* item = world->getComponent<ItemComponent>(armor)) {
            item->weight = 8.0f;
        }

        invSystem.addItem(player, sword);
        invSystem.addItem(player, armor);

        float totalWeight = invSystem.getTotalWeight(player);
        REQUIRE(totalWeight == Approx(11.5f));
    }

    SECTION("Encumbrance check") {
        auto player = factory.createPlayer(10, 10);

        // Set carry capacity based on strength
        if (auto* stats = world->getComponent<StatsComponent>(player)) {
            stats->strength = 10;
        }

        auto heavyArmor = factory.createItem("armor_plate", 0, 0);
        if (auto* item = world->getComponent<ItemComponent>(heavyArmor)) {
            item->weight = 50.0f;
        }

        invSystem.addItem(player, heavyArmor);

        bool encumbered = invSystem.isEncumbered(player);
        REQUIRE(encumbered == true);

        // Check movement penalty
        float penalty = invSystem.getEncumbrancePenalty(player);
        REQUIRE(penalty > 0.0f);
    }

    SECTION("Cannot add item when overweight") {
        auto player = factory.createPlayer(10, 10);
        auto* inventory = world->getComponent<InventoryComponent>(player);

        if (inventory) {
            inventory->maxWeight = 10.0f;
        }

        auto lightItem = factory.createItem("potion_minor", 0, 0);
        auto heavyItem = factory.createItem("armor_plate", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(lightItem)) {
            item->weight = 0.5f;
        }
        if (auto* item = world->getComponent<ItemComponent>(heavyItem)) {
            item->weight = 20.0f;
        }

        REQUIRE(invSystem.addItem(player, lightItem) == true);
        REQUIRE(invSystem.canAddItem(player, heavyItem) == false);
        REQUIRE(invSystem.addItem(player, heavyItem) == false);
    }
}

TEST_CASE("InventorySystem sorting and filtering", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableInventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Sort by type") {
        auto player = factory.createPlayer(10, 10);

        auto weapon = factory.createItem("sword_basic", 0, 0);
        auto armor = factory.createItem("armor_leather", 0, 0);
        auto potion = factory.createItem("potion_minor", 0, 0);

        invSystem.addItem(player, potion);
        invSystem.addItem(player, weapon);
        invSystem.addItem(player, armor);

        invSystem.sortInventory(player, SortType::ByType);

        auto items = invSystem.getItems(player);
        // Items should be sorted by type
        bool sorted = true;
        for (size_t i = 1; i < items.size(); ++i) {
            auto* item1 = world->getComponent<ItemComponent>(items[i-1]);
            auto* item2 = world->getComponent<ItemComponent>(items[i]);
            if (item1 && item2 && item1->type > item2->type) {
                sorted = false;
                break;
            }
        }
        REQUIRE(sorted == true);
    }

    SECTION("Sort by value") {
        auto player = factory.createPlayer(10, 10);

        auto cheap = factory.createItem("potion_minor", 0, 0);
        auto medium = factory.createItem("sword_basic", 0, 0);
        auto expensive = factory.createItem("sword_enchanted", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(cheap)) item->value = 10;
        if (auto* item = world->getComponent<ItemComponent>(medium)) item->value = 100;
        if (auto* item = world->getComponent<ItemComponent>(expensive)) item->value = 1000;

        invSystem.addItem(player, medium);
        invSystem.addItem(player, expensive);
        invSystem.addItem(player, cheap);

        invSystem.sortInventory(player, SortType::ByValue);

        auto items = invSystem.getItems(player);
        // Should be sorted by value descending
        if (items.size() == 3) {
            auto* item1 = world->getComponent<ItemComponent>(items[0]);
            auto* item3 = world->getComponent<ItemComponent>(items[2]);
            REQUIRE(item1->value >= item3->value);
        }
    }

    SECTION("Filter by type") {
        auto player = factory.createPlayer(10, 10);

        invSystem.addItem(player, factory.createItem("sword_basic", 0, 0));
        invSystem.addItem(player, factory.createItem("potion_minor", 0, 0));
        invSystem.addItem(player, factory.createItem("armor_leather", 0, 0));
        invSystem.addItem(player, factory.createItem("potion_major", 0, 0));

        auto potions = invSystem.getItemsByType(player, ItemType::Consumable);
        REQUIRE(potions.size() == 2);

        auto weapons = invSystem.getItemsByType(player, ItemType::Weapon);
        REQUIRE(weapons.size() == 1);
    }

    SECTION("Search items by name") {
        auto player = factory.createPlayer(10, 10);

        auto sword = factory.createItem("sword_basic", 0, 0);
        auto potion1 = factory.createItem("potion_minor", 0, 0);
        auto potion2 = factory.createItem("potion_major", 0, 0);

        invSystem.addItem(player, sword);
        invSystem.addItem(player, potion1);
        invSystem.addItem(player, potion2);

        auto results = invSystem.findItemsByName(player, "potion");
        REQUIRE(results.size() == 2);

        results = invSystem.findItemsByName(player, "sword");
        REQUIRE(results.size() == 1);
    }
}

TEST_CASE("InventorySystem container interactions", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    InventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Open container") {
        auto player = factory.createPlayer(10, 10);
        auto chest = factory.createEntity();
        world->addComponent<InventoryComponent>(chest, 30);
        world->addComponent<PositionComponent>(chest, 10, 10);

        bool opened = invSystem.openContainer(player, chest);
        REQUIRE(opened == true);
    }

    SECTION("Transfer all items") {
        auto player = factory.createPlayer(10, 10);
        auto chest = factory.createEntity();
        world->addComponent<InventoryComponent>(chest, 30);

        for (int i = 0; i < 5; ++i) {
            invSystem.addItem(player, factory.createItem("potion_minor", 0, 0));
        }

        int transferred = invSystem.transferAll(player, chest);
        REQUIRE(transferred == 5);
        REQUIRE(invSystem.getItemCount(player) == 0);
        REQUIRE(invSystem.getItemCount(chest) == 5);
    }

    SECTION("Loot all from container") {
        auto player = factory.createPlayer(10, 10);
        auto corpse = factory.createEntity();
        world->addComponent<InventoryComponent>(corpse, 10);

        invSystem.addItem(corpse, factory.createItem("sword_basic", 0, 0));
        invSystem.addItem(corpse, factory.createItem("gold_coins", 0, 0));
        invSystem.addItem(corpse, factory.createItem("potion_minor", 0, 0));

        int looted = invSystem.lootAll(player, corpse);
        REQUIRE(looted == 3);
        REQUIRE(invSystem.getItemCount(corpse) == 0);
        REQUIRE(invSystem.getItemCount(player) == 3);
    }

    SECTION("Auto-loot valuable items") {
        auto player = factory.createPlayer(10, 10);
        auto corpse = factory.createEntity();
        world->addComponent<InventoryComponent>(corpse, 10);

        auto junk = factory.createItem("bone", 0, 0);
        auto valuable = factory.createItem("gold_coins", 0, 0);
        auto rare = factory.createItem("sword_enchanted", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(junk)) item->value = 1;
        if (auto* item = world->getComponent<ItemComponent>(valuable)) item->value = 100;
        if (auto* item = world->getComponent<ItemComponent>(rare)) item->value = 1000;

        invSystem.addItem(corpse, junk);
        invSystem.addItem(corpse, valuable);
        invSystem.addItem(corpse, rare);

        int looted = invSystem.autoLoot(player, corpse, 50); // Min value 50
        REQUIRE(looted == 2); // Gold and sword
        REQUIRE(invSystem.hasItem(corpse, junk) == true);
        REQUIRE(invSystem.hasItem(player, valuable) == true);
        REQUIRE(invSystem.hasItem(player, rare) == true);
    }
}

TEST_CASE("InventorySystem special items", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    InventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Quest items cannot be dropped") {
        auto player = factory.createPlayer(10, 10);
        auto questItem = factory.createItem("quest_artifact", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(questItem)) {
            item->isQuest = true;
        }

        invSystem.addItem(player, questItem);

        bool dropped = invSystem.dropItem(player, questItem, 15, 15);
        REQUIRE(dropped == false);
        REQUIRE(invSystem.hasItem(player, questItem) == true);
    }

    SECTION("Unique items prevent duplicates") {
        auto player = factory.createPlayer(10, 10);

        auto unique1 = factory.createItem("artifact_unique", 0, 0);
        auto unique2 = factory.createItem("artifact_unique", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(unique1)) {
            item->isUnique = true;
        }
        if (auto* item = world->getComponent<ItemComponent>(unique2)) {
            item->isUnique = true;
        }

        REQUIRE(invSystem.addItem(player, unique1) == true);
        REQUIRE(invSystem.addItem(player, unique2) == false);
    }

    SECTION("Bound items transfer with owner") {
        auto player = factory.createPlayer(10, 10);
        auto chest = factory.createEntity();
        world->addComponent<InventoryComponent>(chest, 10);

        auto boundItem = factory.createItem("ring_binding", 0, 0);
        if (auto* item = world->getComponent<ItemComponent>(boundItem)) {
            item->isBound = true;
            item->boundTo = player;
        }

        invSystem.addItem(player, boundItem);

        // Cannot transfer bound items
        bool transferred = invSystem.transferItem(boundItem, player, chest);
        REQUIRE(transferred == false);
        REQUIRE(invSystem.hasItem(player, boundItem) == true);
    }

    SECTION("Perishable items decay") {
        auto player = factory.createPlayer(10, 10);
        auto food = factory.createItem("food_apple", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(food)) {
            item->isPerishable = true;
            item->freshness = 1.0f;
        }

        invSystem.addItem(player, food);

        // Update decay over time
        for (int i = 0; i < 100; ++i) {
            invSystem.updatePerishables(player, 1.0f);
        }

        if (auto* item = world->getComponent<ItemComponent>(food)) {
            REQUIRE(item->freshness < 1.0f);

            // Fully decayed items should be removed
            if (item->freshness <= 0.0f) {
                REQUIRE(invSystem.hasItem(player, food) == false);
            }
        }
    }
}

TEST_CASE("InventorySystem events", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    InventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Item picked up event") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("gold_coins", 10, 10);

        bool eventFired = false;
        invSystem.onItemPickedUp = [&eventFired](Entity* e, Entity* i) {
            eventFired = true;
        };

        invSystem.pickupItem(player, item);
        REQUIRE(eventFired == true);
    }

    SECTION("Item dropped event") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("sword_basic", 0, 0);

        invSystem.addItem(player, item);

        bool eventFired = false;
        invSystem.onItemDropped = [&eventFired](Entity* e, Entity* i) {
            eventFired = true;
        };

        invSystem.dropItem(player, item, 15, 15);
        REQUIRE(eventFired == true);
    }

    SECTION("Item used event") {
        auto player = factory.createPlayer(10, 10);
        auto potion = factory.createItem("potion_minor", 0, 0);

        invSystem.addItem(player, potion);

        bool eventFired = false;
        invSystem.onItemUsed = [&eventFired](Entity* e, Entity* i) {
            eventFired = true;
        };

        invSystem.useItem(player, potion);
        REQUIRE(eventFired == true);
    }

    SECTION("Inventory full event") {
        auto player = factory.createPlayer(10, 10);
        auto* inventory = world->getComponent<InventoryComponent>(player);

        if (inventory) {
            inventory->capacity = 1;
        }

        invSystem.addItem(player, factory.createItem("potion_minor", 0, 0));

        bool eventFired = false;
        invSystem.onInventoryFull = [&eventFired](Entity* e) {
            eventFired = true;
        };

        // Try to add when full
        invSystem.addItem(player, factory.createItem("potion_minor", 0, 0));
        REQUIRE(eventFired == true);
    }
}

TEST_CASE("InventorySystem edge cases", "[inventory][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    InventorySystem invSystem(&testMap, nullptr);
    EntityFactory factory(world.get());

    SECTION("Null entity operations") {
        auto item = factory.createItem("potion_minor", 0, 0);

        REQUIRE(invSystem.addItem(nullptr, item) == false);
        REQUIRE(invSystem.removeItem(nullptr, item) == false);
        REQUIRE(invSystem.hasItem(nullptr, item) == false);
        REQUIRE(invSystem.getItems(nullptr).empty());
    }

    SECTION("Null item operations") {
        auto player = factory.createPlayer(10, 10);

        REQUIRE(invSystem.addItem(player, nullptr) == false);
        REQUIRE(invSystem.removeItem(player, nullptr) == false);
        REQUIRE(invSystem.hasItem(player, nullptr) == false);
    }

    SECTION("Entity without inventory component") {
        auto entity = factory.createEntity();
        auto item = factory.createItem("potion_minor", 0, 0);

        REQUIRE(invSystem.addItem(entity, item) == false);
        REQUIRE(invSystem.getItemCount(entity) == 0);
    }

    SECTION("Remove item not in inventory") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("sword_basic", 0, 0);

        REQUIRE(invSystem.removeItem(player, item) == false);
    }

    SECTION("Transfer to same inventory") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("potion_minor", 0, 0);

        invSystem.addItem(player, item);

        bool transferred = invSystem.transferItem(item, player, player);
        REQUIRE(transferred == false); // Should fail or no-op
    }

    SECTION("Split stack larger than size") {
        auto player = factory.createPlayer(10, 10);
        auto coins = factory.createItem("gold_coins", 0, 0);

        if (auto* item = world->getComponent<ItemComponent>(coins)) {
            item->isStackable = true;
            item->stackSize = 50;
        }

        invSystem.addItem(player, coins);

        auto split = invSystem.splitStack(player, coins, 100);
        REQUIRE(split == nullptr);

        // Original stack unchanged
        if (auto* item = world->getComponent<ItemComponent>(coins)) {
            REQUIRE(item->stackSize == 50);
        }
    }

    SECTION("Use non-consumable item") {
        auto player = factory.createPlayer(10, 10);
        auto sword = factory.createItem("sword_basic", 0, 0);

        invSystem.addItem(player, sword);

        bool used = invSystem.useItem(player, sword);
        REQUIRE(used == false);
        REQUIRE(invSystem.hasItem(player, sword) == true);
    }

    SECTION("Drop item at invalid location") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("potion_minor", 0, 0);

        invSystem.addItem(player, item);

        // Drop at wall location
        testMap.getTile(15, 15).type = TileType::Wall;
        bool dropped = invSystem.dropItem(player, item, 15, 15);

        REQUIRE(dropped == false);
        REQUIRE(invSystem.hasItem(player, item) == true);
    }

    SECTION("Pickup item from distance") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("potion_minor", 50, 50); // Far away

        bool picked = invSystem.pickupItem(player, item);
        REQUIRE(picked == false);
    }

    SECTION("Zero capacity inventory") {
        auto player = factory.createPlayer(10, 10);
        auto* inventory = world->getComponent<InventoryComponent>(player);

        if (inventory) {
            inventory->capacity = 0;
        }

        auto item = factory.createItem("potion_minor", 0, 0);
        REQUIRE(invSystem.addItem(player, item) == false);
    }

    SECTION("Negative weight handling") {
        auto player = factory.createPlayer(10, 10);
        auto item = factory.createItem("balloon_helium", 0, 0);

        if (auto* itemComp = world->getComponent<ItemComponent>(item)) {
            itemComp->weight = -1.0f; // Negative weight
        }

        invSystem.addItem(player, item);

        float weight = invSystem.getTotalWeight(player);
        // Should handle negative weight gracefully
        REQUIRE(weight >= 0.0f);
    }
}