#include <catch2/catch_test_macros.hpp>
#include "inventory.h"
#include "item.h"
#include "item_factory.h"
#include "player.h"
#include "config.h"

TEST_CASE("Inventory basic operations", "[inventory]") {
    Inventory inventory(10);  // Create inventory with 10 slots

    SECTION("Empty inventory") {
        REQUIRE(inventory.getUsedSlots() == 0);
        REQUIRE(inventory.getTotalSlots() == 10);
        REQUIRE(!inventory.isFull());
        REQUIRE(inventory.hasSpace());
        REQUIRE(inventory.hasSpace(5));
        REQUIRE(inventory.getTotalWeight() == 0);
    }

    SECTION("Add and remove items") {
        auto item1 = std::make_unique<Item>("sword");
        item1->name = "Iron Sword";
        item1->type = Item::ItemType::WEAPON;
        item1->weight = 5;
        item1->stackable = false;

        // Add item
        REQUIRE(inventory.addItem(std::move(item1)));
        REQUIRE(inventory.getUsedSlots() == 1);
        REQUIRE(!inventory.isFull());

        // Get item
        Item* retrieved = inventory.getItem(0);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->name == "Iron Sword");

        // Remove item
        auto removed = inventory.removeItem(0);
        REQUIRE(removed != nullptr);
        REQUIRE(removed->name == "Iron Sword");
        REQUIRE(inventory.getUsedSlots() == 0);
    }

    SECTION("Capacity limits") {
        // Fill inventory
        for (int i = 0; i < 10; i++) {
            auto item = std::make_unique<Item>("item_" + std::to_string(i));
            item->stackable = false;
            REQUIRE(inventory.addItem(std::move(item)));
        }

        REQUIRE(inventory.isFull());
        REQUIRE(!inventory.hasSpace());
        REQUIRE(inventory.getUsedSlots() == 10);

        // Try to add one more
        auto extra = std::make_unique<Item>("extra");
        extra->stackable = false;
        REQUIRE(!inventory.addItem(std::move(extra)));
        REQUIRE(inventory.getUsedSlots() == 10);
    }

    SECTION("Find items") {
        auto potion = std::make_unique<Item>("potion_minor");
        potion->name = "Minor Healing Potion";
        potion->type = Item::ItemType::POTION;
        potion->stackable = false;

        auto scroll = std::make_unique<Item>("scroll_identify");
        scroll->name = "Scroll of Identify";
        scroll->type = Item::ItemType::SCROLL;
        scroll->stackable = false;

        inventory.addItem(std::move(potion));
        inventory.addItem(std::move(scroll));

        // Find by ID
        Item* found = inventory.findItem("potion_minor");
        REQUIRE(found != nullptr);
        REQUIRE(found->name == "Minor Healing Potion");

        // Find by type
        auto potions = inventory.findItems(Item::ItemType::POTION);
        REQUIRE(potions.size() == 1);
        REQUIRE(potions[0]->id == "potion_minor");

        // Find slot
        int slot = inventory.findSlot(found);
        REQUIRE(slot == 0);
    }

    SECTION("Remove by pointer") {
        auto item = std::make_unique<Item>("test");
        Item* item_ptr = item.get();
        inventory.addItem(std::move(item));

        auto removed = inventory.removeItem(item_ptr);
        REQUIRE(removed != nullptr);
        REQUIRE(removed->id == "test");
        REQUIRE(inventory.getUsedSlots() == 0);
    }
}

TEST_CASE("Inventory stacking", "[inventory]") {
    Inventory inventory;

    SECTION("Stack identical stackable items") {
        auto arrows1 = std::make_unique<Item>("arrows");
        arrows1->name = "Arrows";
        arrows1->stackable = true;
        arrows1->stack_size = 10;
        arrows1->max_stack = 50;

        auto arrows2 = std::make_unique<Item>("arrows");
        arrows2->name = "Arrows";
        arrows2->stackable = true;
        arrows2->stack_size = 15;
        arrows2->max_stack = 50;

        REQUIRE(inventory.addItem(std::move(arrows1)));
        REQUIRE(inventory.getUsedSlots() == 1);

        // Second set should stack with first
        REQUIRE(inventory.addItem(std::move(arrows2)));
        REQUIRE(inventory.getUsedSlots() == 1);  // Still just one slot

        Item* stacked = inventory.getItem(0);
        REQUIRE(stacked != nullptr);
        REQUIRE(stacked->stack_size == 25);  // 10 + 15
    }

    SECTION("Respect max stack size") {
        auto gold1 = std::make_unique<Item>("gold");
        gold1->stackable = true;
        gold1->stack_size = 45;
        gold1->max_stack = 50;

        auto gold2 = std::make_unique<Item>("gold");
        gold2->stackable = true;
        gold2->stack_size = 10;
        gold2->max_stack = 50;

        REQUIRE(inventory.addItem(std::move(gold1)));
        REQUIRE(inventory.addItem(std::move(gold2)));

        // Should create two stacks: one with 50, one with 5
        REQUIRE(inventory.getUsedSlots() == 2);

        auto items = inventory.getAllItems();
        int total = 0;
        for (auto* item : items) {
            total += item->stack_size;
        }
        REQUIRE(total == 55);  // 45 + 10
    }

    SECTION("Non-stackable items don't stack") {
        auto sword1 = std::make_unique<Item>("sword");
        sword1->stackable = false;

        auto sword2 = std::make_unique<Item>("sword");
        sword2->stackable = false;

        REQUIRE(inventory.addItem(std::move(sword1)));
        REQUIRE(inventory.addItem(std::move(sword2)));
        REQUIRE(inventory.getUsedSlots() == 2);  // Two separate slots
    }

    SECTION("Can check if item can stack") {
        auto arrows = std::make_unique<Item>("arrows");
        arrows->stackable = true;
        arrows->stack_size = 10;
        arrows->max_stack = 50;
        inventory.addItem(std::move(arrows));

        auto more_arrows = std::make_unique<Item>("arrows");
        more_arrows->stackable = true;
        REQUIRE(inventory.canStackWith(more_arrows.get()));

        auto sword = std::make_unique<Item>("sword");
        sword->stackable = false;
        REQUIRE(!inventory.canStackWith(sword.get()));
    }
}

TEST_CASE("Inventory weight management", "[inventory]") {
    Inventory inventory;

    SECTION("Track total weight") {
        auto sword = std::make_unique<Item>("sword");
        sword->weight = 10;
        sword->stackable = false;

        auto armor = std::make_unique<Item>("armor");
        armor->weight = 20;
        armor->stackable = false;

        inventory.addItem(std::move(sword));
        inventory.addItem(std::move(armor));

        REQUIRE(inventory.getTotalWeight() == 30);
    }

    SECTION("Weight with stacks") {
        auto arrows = std::make_unique<Item>("arrows");
        arrows->weight = 1;  // Per arrow
        arrows->stackable = true;
        arrows->stack_size = 20;

        inventory.addItem(std::move(arrows));
        REQUIRE(inventory.getTotalWeight() == 20);  // 1 * 20
    }
}

TEST_CASE("Inventory utility functions", "[inventory]") {
    Inventory inventory(5);

    SECTION("Clear inventory") {
        for (int i = 0; i < 3; i++) {
            auto item = std::make_unique<Item>("item_" + std::to_string(i));
            inventory.addItem(std::move(item));
        }

        REQUIRE(inventory.getUsedSlots() == 3);
        inventory.clear();
        REQUIRE(inventory.getUsedSlots() == 0);
        REQUIRE(inventory.getTotalWeight() == 0);
    }

    SECTION("Sort inventory") {
        auto weapon = std::make_unique<Item>("sword");
        weapon->type = Item::ItemType::WEAPON;
        weapon->name = "Sword";

        auto potion = std::make_unique<Item>("potion");
        potion->type = Item::ItemType::POTION;
        potion->name = "Potion";

        auto armor = std::make_unique<Item>("armor");
        armor->type = Item::ItemType::ARMOR;
        armor->name = "Armor";

        // Add in mixed order
        inventory.addItem(std::move(weapon));
        inventory.addItem(std::move(potion));
        inventory.addItem(std::move(armor));

        inventory.sort();

        // Check sorted by type (POTION=0, ARMOR=3, WEAPON=2)
        auto items = inventory.getAllItems();
        REQUIRE(items[0]->type == Item::ItemType::POTION);
        REQUIRE(items[1]->type == Item::ItemType::WEAPON);
        REQUIRE(items[2]->type == Item::ItemType::ARMOR);
    }

    SECTION("Get all items") {
        auto item1 = std::make_unique<Item>("item1");
        auto item2 = std::make_unique<Item>("item2");

        inventory.addItem(std::move(item1));
        inventory.addItem(std::move(item2));

        auto all_items = inventory.getAllItems();
        REQUIRE(all_items.size() == 2);
        REQUIRE(all_items[0]->id == "item1");
        REQUIRE(all_items[1]->id == "item2");
    }
}

TEST_CASE("Player inventory integration", "[inventory]") {
    // Ensure config is loaded
    Config::getInstance().loadFromFile("config.yml");
    ItemFactory::getInstance().loadFromJson(Config::getInstance().getDataFilePath("items.json"));

    Player player(5, 5);

    SECTION("Player has inventory") {
        REQUIRE(player.inventory != nullptr);
        REQUIRE(player.canPickUp());
    }

    SECTION("Pickup regular items") {
        auto potion = ItemFactory::getInstance().create("potion_minor");
        REQUIRE(potion != nullptr);

        REQUIRE(player.pickupItem(std::move(potion)));
        REQUIRE(player.hasItem("potion_minor"));
        REQUIRE(player.countItems("potion_minor") == 1);
    }

    SECTION("Gold goes to gold counter") {
        int initial_gold = player.gold;

        auto gold = ItemFactory::getInstance().create("gold");
        gold->properties["amount"] = 50;

        REQUIRE(player.pickupItem(std::move(gold)));
        REQUIRE(player.gold == initial_gold + 50);

        // Gold shouldn't be in inventory
        REQUIRE(!player.hasItem("gold_coins"));
    }

    SECTION("Inventory full check") {
        // Fill player's inventory
        for (int i = 0; i < 26; i++) {  // Default capacity
            auto item = std::make_unique<Item>("item_" + std::to_string(i));
            item->stackable = false;
            player.pickupItem(std::move(item));
        }

        REQUIRE(!player.canPickUp());

        // Try to pick up one more
        auto extra = std::make_unique<Item>("extra");
        REQUIRE(!player.pickupItem(std::move(extra)));
    }

    SECTION("Drop items") {
        auto item = std::make_unique<Item>("test_item");
        item->name = "Test Item";
        player.pickupItem(std::move(item));

        REQUIRE(player.hasItem("test_item"));
        REQUIRE(player.dropItem(0));
        REQUIRE(!player.hasItem("test_item"));
    }

    // Cleanup
    ItemFactory::cleanup();
}