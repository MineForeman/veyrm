#include <catch2/catch_test_macros.hpp>
#include "item.h"
#include "item_factory.h"
#include "item_manager.h"
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

TEST_CASE("ItemFactory", "[item]") {
    // Ensure config is loaded (tests run from project root thanks to build.sh)
    Config::getInstance().loadFromFile("config.yml");

    ItemFactory& factory = ItemFactory::getInstance();

    SECTION("Load JSON data") {
        // Load test data
        factory.loadFromJson(Config::getInstance().getDataFilePath("items.json"));

        REQUIRE(factory.hasTemplate("potion_minor"));
        REQUIRE(factory.hasTemplate("gold_coins"));
        REQUIRE(factory.hasTemplate("ration"));
    }

    SECTION("Create items from templates") {
        factory.loadFromJson(Config::getInstance().getDataFilePath("items.json"));

        auto potion = factory.create("potion_minor");
        REQUIRE(potion != nullptr);
        REQUIRE(potion->id == "potion_minor");
        REQUIRE(potion->name == "Minor Healing Potion");
        REQUIRE(potion->symbol == '!');
        REQUIRE(potion->type == Item::ItemType::POTION);

        auto gold = factory.create("gold_coins");
        REQUIRE(gold != nullptr);
        REQUIRE(gold->id == "gold_coins");
        REQUIRE(gold->type == Item::ItemType::GOLD);
        REQUIRE(gold->stackable == true);
    }

    SECTION("Get items for depth") {
        factory.loadFromJson(Config::getInstance().getDataFilePath("items.json"));

        auto depth1_items = factory.getItemsForDepth(1);
        REQUIRE(!depth1_items.empty());

        // Minor potions should be available at depth 1
        bool has_minor_potion = false;
        for (const auto& id : depth1_items) {
            if (id == "potion_minor") {
                has_minor_potion = true;
                break;
            }
        }
        REQUIRE(has_minor_potion);
    }

    // Cleanup
    ItemFactory::cleanup();
}

TEST_CASE("ItemManager", "[item]") {
    // Ensure config is loaded (tests run from project root)
    Config::getInstance().loadFromFile("config.yml");

    Map map(50, 50);
    ItemManager manager(&map);

    // Initialize map with walkable tiles
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 50; x++) {
            map.setTile(x, y, TileType::FLOOR);
        }
    }

    // Load item data
    ItemFactory::getInstance().loadFromJson(Config::getInstance().getDataFilePath("items.json"));

    SECTION("Spawn and retrieve items") {
        manager.spawnItem("potion_minor", 10, 10);

        auto item = manager.getItemAt(10, 10);
        REQUIRE(item != nullptr);
        REQUIRE(item->id == "potion_minor");
        REQUIRE(item->x == 10);
        REQUIRE(item->y == 10);

        REQUIRE(manager.getItemCount() == 1);
    }

    SECTION("Spawn gold") {
        manager.spawnGold(5, 5, 50);

        auto item = manager.getItemAt(5, 5);
        REQUIRE(item != nullptr);
        REQUIRE(item->type == Item::ItemType::GOLD);
        REQUIRE(item->properties["amount"] == 50);
    }

    SECTION("Remove items") {
        manager.spawnItem("ration", 15, 15);
        REQUIRE(manager.getItemCount() == 1);

        auto item = manager.getItemAt(15, 15);
        REQUIRE(item != nullptr);

        manager.removeItem(item);
        REQUIRE(manager.getItemCount() == 0);
        REQUIRE(manager.getItemAt(15, 15) == nullptr);
    }

    SECTION("Multiple items at location") {
        manager.spawnItem("potion_minor", 20, 20);
        manager.spawnItem("scroll_identify", 20, 20);

        auto items = manager.getItemsAt(20, 20);
        REQUIRE(items.size() == 2);
    }

    SECTION("Clear all items") {
        manager.spawnItem("potion_minor", 1, 1);
        manager.spawnItem("ration", 2, 2);
        manager.spawnGold(3, 3, 100);

        REQUIRE(manager.getItemCount() == 3);

        manager.clear();
        REQUIRE(manager.getItemCount() == 0);
    }

    SECTION("Invalid spawn positions") {
        // Outside map bounds
        manager.spawnItem("potion_minor", -1, -1);
        REQUIRE(manager.getItemCount() == 0);

        manager.spawnItem("potion_minor", 100, 100);
        REQUIRE(manager.getItemCount() == 0);

        // Non-walkable tile
        map.setTile(25, 25, TileType::WALL);
        manager.spawnItem("potion_minor", 25, 25);
        REQUIRE(manager.getItemCount() == 0);
    }

    // Cleanup
    ItemFactory::cleanup();
}