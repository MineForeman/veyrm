#include <catch2/catch_test_macros.hpp>
#include "item.h"
// #include "item_factory.h"  // Legacy - removed
// #include "item_manager.h"  // Legacy - removed
#include "map.h"
#include "config.h"

TEST_CASE("Item creation and properties", "[item]") {
    SECTION("Default constructor") {
        Item item;
        REQUIRE(item.x == 0);
        REQUIRE(item.y == 0);
        REQUIRE(item.id == "");
        REQUIRE(item.name == "Unknown");
        REQUIRE(item.symbol == '?');
        REQUIRE(item.type == Item::ItemType::MISC);
    }

    SECTION("Constructor with ID") {
        Item item("test_item");
        REQUIRE(item.id == "test_item");
        REQUIRE(item.name == "Unknown");
    }

    SECTION("Position setting") {
        Item item;
        item.setPosition(5, 10);
        REQUIRE(item.x == 5);
        REQUIRE(item.y == 10);
    }

    SECTION("Type conversion") {
        REQUIRE(Item::stringToType("potion") == Item::ItemType::POTION);
        REQUIRE(Item::stringToType("scroll") == Item::ItemType::SCROLL);
        REQUIRE(Item::stringToType("weapon") == Item::ItemType::WEAPON);
        REQUIRE(Item::stringToType("armor") == Item::ItemType::ARMOR);
        REQUIRE(Item::stringToType("food") == Item::ItemType::FOOD);
        REQUIRE(Item::stringToType("gold") == Item::ItemType::GOLD);
        REQUIRE(Item::stringToType("unknown") == Item::ItemType::MISC);

        REQUIRE(Item::typeToString(Item::ItemType::POTION) == "potion");
        REQUIRE(Item::typeToString(Item::ItemType::GOLD) == "gold");
    }
}

TEST_CASE("Item stacking", "[item]") {
    SECTION("Stackable items") {
        Item item1("gold");
        item1.stackable = true;
        item1.max_stack = 100;
        item1.stack_size = 10;

        Item item2("gold");
        item2.stackable = true;
        item2.max_stack = 100;
        item2.stack_size = 5;

        REQUIRE(item1.canStackWith(item2));

        REQUIRE(item1.addToStack(20));
        REQUIRE(item1.stack_size == 30);

        REQUIRE(item1.removeFromStack(15));
        REQUIRE(item1.stack_size == 15);
    }

    SECTION("Non-stackable items") {
        Item item1("sword");
        item1.stackable = false;

        Item item2("sword");
        item2.stackable = false;

        REQUIRE_FALSE(item1.canStackWith(item2));
        REQUIRE_FALSE(item1.addToStack(1));
    }

    SECTION("Stack limits") {
        Item item("arrows");
        item.stackable = true;
        item.max_stack = 20;
        item.stack_size = 18;

        REQUIRE(item.addToStack(2));
        REQUIRE(item.stack_size == 20);
        REQUIRE_FALSE(item.addToStack(1));  // Exceeds max

        REQUIRE_FALSE(item.removeFromStack(25));  // More than available
        REQUIRE(item.removeFromStack(10));
        REQUIRE(item.stack_size == 10);
    }
}

// ItemFactory tests removed - using ECS DataLoader
/*TEST_CASE("ItemFactory", "[item]") {
    // Tests removed - ItemFactory has been replaced by ECS DataLoader
}*/

// ItemManager tests removed - using ECS item system
/*TEST_CASE("ItemManager", "[item]") {
    // Tests removed - ItemManager has been replaced by ECS item system
}*/