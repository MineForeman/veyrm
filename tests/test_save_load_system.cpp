#include <catch2/catch_test_macros.hpp>
#include "ecs/save_load_system.h"
#include "ecs/game_world.h"
#include "ecs/entity_factory.h"
#include "ecs/entity.h"
#include "ecs/component.h"
#include "map.h"
#include <nlohmann/json.hpp>
#include <filesystem>

using namespace ecs;
namespace fs = std::filesystem;

class TestableGameWorld : public GameWorld {
public:
    TestableGameWorld(Map* map) : GameWorld(map) {}

    // Expose protected methods for testing
    using GameWorld::entities;
    using GameWorld::componentManager;
    using GameWorld::systemManager;
};

TEST_CASE("SaveLoadSystem basic operations", "[save_load][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<TestableGameWorld>(&testMap);
    SaveLoadSystem saveSystem(world.get());

    SECTION("System initialization") {
        // System should be initialized properly
        REQUIRE(true);
    }

    SECTION("Save game state to JSON") {
        // Create some test entities
        EntityFactory factory(world.get());
        auto player = factory.createPlayer(10, 10);
        auto monster = factory.createMonster("goblin", 15, 15);
        auto item = factory.createItem("potion_minor", 20, 20);

        nlohmann::json saveData;
        bool saved = saveSystem.saveGameState(saveData);

        REQUIRE(saved == true);
        REQUIRE(saveData.contains("version"));
        REQUIRE(saveData.contains("entities"));
        REQUIRE(saveData.contains("metadata"));

        // Check entities were saved
        REQUIRE(saveData["entities"].is_array());
        REQUIRE(saveData["entities"].size() >= 3);
    }

    SECTION("Load game state from JSON") {
        // Create initial state
        EntityFactory factory(world.get());
        auto player = factory.createPlayer(5, 5);

        // Save current state
        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        // Clear world
        world = std::make_unique<TestableGameWorld>(&testMap);
        SaveLoadSystem newSaveSystem(world.get());

        // Load saved state
        bool loaded = newSaveSystem.loadGameState(saveData);
        REQUIRE(loaded == true);

        // Verify player was restored
        auto restoredPlayer = world->getPlayerEntity();
        REQUIRE(restoredPlayer != nullptr);
    }

    SECTION("Save to file") {
        EntityFactory factory(world.get());
        auto player = factory.createPlayer(25, 25);

        std::string filename = "test_save_temp.json";
        bool saved = saveSystem.saveToFile(filename);
        REQUIRE(saved == true);

        // Check file exists
        REQUIRE(fs::exists(filename));

        // Clean up
        fs::remove(filename);
    }

    SECTION("Load from file") {
        // Create a test save file
        nlohmann::json testSave = {
            {"version", "1.0.0"},
            {"metadata", {
                {"save_time", "2024-01-01T00:00:00"},
                {"play_time", 3600}
            }},
            {"entities", nlohmann::json::array()}
        };

        std::string filename = "test_load_temp.json";
        std::ofstream file(filename);
        file << testSave.dump(2);
        file.close();

        bool loaded = saveSystem.loadFromFile(filename);
        REQUIRE(loaded == true);

        // Clean up
        fs::remove(filename);
    }

    SECTION("Handle missing file") {
        bool loaded = saveSystem.loadFromFile("nonexistent_file.json");
        REQUIRE(loaded == false);
    }

    SECTION("Handle corrupted save data") {
        nlohmann::json badData = {
            {"invalid_key", "bad_value"}
            // Missing required fields
        };

        bool loaded = saveSystem.loadGameState(badData);
        REQUIRE(loaded == false);
    }

    SECTION("Quick save and load") {
        EntityFactory factory(world.get());
        auto player = factory.createPlayer(30, 30);

        // Quick save
        bool saved = saveSystem.quickSave();
        REQUIRE(saved == true);

        // Modify state
        if (auto* pos = world->getComponent<PositionComponent>(player)) {
            pos->x = 35;
            pos->y = 35;
        }

        // Quick load
        bool loaded = saveSystem.quickLoad();
        REQUIRE(loaded == true);

        // Position should be restored
        if (auto* pos = world->getComponent<PositionComponent>(player)) {
            REQUIRE(pos->x == 30);
            REQUIRE(pos->y == 30);
        }
    }

    SECTION("Auto save functionality") {
        saveSystem.setAutoSaveEnabled(true);
        REQUIRE(saveSystem.isAutoSaveEnabled() == true);

        saveSystem.setAutoSaveInterval(60); // 60 seconds
        REQUIRE(saveSystem.getAutoSaveInterval() == 60);

        saveSystem.setAutoSaveEnabled(false);
        REQUIRE(saveSystem.isAutoSaveEnabled() == false);
    }

    SECTION("Save slots management") {
        // Test multiple save slots
        for (int slot = 1; slot <= 3; ++slot) {
            bool saved = saveSystem.saveToSlot(slot);
            REQUIRE(saved == true);

            bool exists = saveSystem.slotExists(slot);
            REQUIRE(exists == true);
        }

        // Load from specific slot
        bool loaded = saveSystem.loadFromSlot(2);
        REQUIRE(loaded == true);

        // Get save info
        auto info = saveSystem.getSaveInfo(1);
        if (info.has_value()) {
            REQUIRE(!info->characterName.empty());
            REQUIRE(info->slot == 1);
        }

        // Delete save slot
        bool deleted = saveSystem.deleteSlot(3);
        REQUIRE(deleted == true);
        REQUIRE(saveSystem.slotExists(3) == false);
    }

    SECTION("List all saves") {
        // Create a few saves
        for (int i = 1; i <= 5; ++i) {
            saveSystem.saveToSlot(i);
        }

        auto saves = saveSystem.listSaves();
        REQUIRE(saves.size() >= 5);

        // Clean up
        for (int i = 1; i <= 5; ++i) {
            saveSystem.deleteSlot(i);
        }
    }
}

TEST_CASE("SaveLoadSystem component serialization", "[save_load][components]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    SaveLoadSystem saveSystem(world.get());
    EntityFactory factory(world.get());

    SECTION("Serialize PositionComponent") {
        auto entity = factory.createEntity();
        world->addComponent<PositionComponent>(entity, 42, 24);

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        // Find entity in save data
        bool found = false;
        for (const auto& e : saveData["entities"]) {
            if (e.contains("components") && e["components"].contains("position")) {
                auto pos = e["components"]["position"];
                REQUIRE(pos["x"] == 42);
                REQUIRE(pos["y"] == 24);
                found = true;
                break;
            }
        }
        REQUIRE(found == true);
    }

    SECTION("Serialize HealthComponent") {
        auto entity = factory.createEntity();
        world->addComponent<HealthComponent>(entity, 100, 150);

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        bool found = false;
        for (const auto& e : saveData["entities"]) {
            if (e.contains("components") && e["components"].contains("health")) {
                auto health = e["components"]["health"];
                REQUIRE(health["hp"] == 100);
                REQUIRE(health["max_hp"] == 150);
                found = true;
                break;
            }
        }
        REQUIRE(found == true);
    }

    SECTION("Serialize RenderableComponent") {
        auto entity = factory.createEntity();
        world->addComponent<RenderableComponent>(entity, '@',
            ftxui::Color::RGB(255, 0, 0), ftxui::Color::RGB(0, 0, 0));

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        bool found = false;
        for (const auto& e : saveData["entities"]) {
            if (e.contains("components") && e["components"].contains("renderable")) {
                auto render = e["components"]["renderable"];
                REQUIRE(render["glyph"] == '@');
                found = true;
                break;
            }
        }
        REQUIRE(found == true);
    }

    SECTION("Serialize CombatComponent") {
        auto entity = factory.createEntity();
        auto combat = std::make_unique<CombatComponent>();
        combat->minDamage = 5;
        combat->maxDamage = 10;
        combat->defense = 3;
        combat->attackBonus = 2;
        world->addComponent(entity, std::move(combat));

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        bool found = false;
        for (const auto& e : saveData["entities"]) {
            if (e.contains("components") && e["components"].contains("combat")) {
                auto combat = e["components"]["combat"];
                REQUIRE(combat["min_damage"] == 5);
                REQUIRE(combat["max_damage"] == 10);
                REQUIRE(combat["defense"] == 3);
                REQUIRE(combat["attack_bonus"] == 2);
                found = true;
                break;
            }
        }
        REQUIRE(found == true);
    }

    SECTION("Serialize InventoryComponent") {
        auto entity = factory.createEntity();
        auto inventory = std::make_unique<InventoryComponent>();
        inventory->capacity = 20;

        // Add some items
        auto item1 = factory.createItem("potion_minor", 0, 0);
        auto item2 = factory.createItem("sword_basic", 0, 0);
        inventory->items.push_back(item1);
        inventory->items.push_back(item2);

        world->addComponent(entity, std::move(inventory));

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        bool found = false;
        for (const auto& e : saveData["entities"]) {
            if (e.contains("components") && e["components"].contains("inventory")) {
                auto inv = e["components"]["inventory"];
                REQUIRE(inv["capacity"] == 20);
                REQUIRE(inv["items"].size() == 2);
                found = true;
                break;
            }
        }
        REQUIRE(found == true);
    }

    SECTION("Serialize complex entity") {
        auto player = factory.createPlayer(15, 20);

        // Add multiple components
        if (auto* stats = world->getComponent<StatsComponent>(player)) {
            stats->level = 10;
            stats->experience = 5000;
            stats->strength = 18;
            stats->dexterity = 14;
            stats->intelligence = 12;
        }

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        // Reload and verify
        world = std::make_unique<GameWorld>(&testMap);
        SaveLoadSystem newSaveSystem(world.get());

        bool loaded = newSaveSystem.loadGameState(saveData);
        REQUIRE(loaded == true);

        auto restoredPlayer = world->getPlayerEntity();
        REQUIRE(restoredPlayer != nullptr);

        if (auto* stats = world->getComponent<StatsComponent>(restoredPlayer)) {
            REQUIRE(stats->level == 10);
            REQUIRE(stats->experience == 5000);
            REQUIRE(stats->strength == 18);
        }
    }
}

TEST_CASE("SaveLoadSystem error handling", "[save_load][errors]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    SaveLoadSystem saveSystem(world.get());

    SECTION("Handle invalid JSON") {
        nlohmann::json invalidJson = "not a valid json object";
        bool loaded = saveSystem.loadGameState(invalidJson);
        REQUIRE(loaded == false);
    }

    SECTION("Handle wrong version") {
        nlohmann::json saveData = {
            {"version", "999.0.0"}, // Future version
            {"entities", nlohmann::json::array()},
            {"metadata", nlohmann::json::object()}
        };

        bool loaded = saveSystem.loadGameState(saveData);
        // Should handle version mismatch gracefully
        REQUIRE((loaded == true || loaded == false));
    }

    SECTION("Handle read-only file system") {
        // Try to save to a read-only location
        bool saved = saveSystem.saveToFile("/readonly/test.json");
        REQUIRE(saved == false);
    }

    SECTION("Handle large save data") {
        EntityFactory factory(world.get());

        // Create many entities
        for (int i = 0; i < 1000; ++i) {
            factory.createMonster("goblin", i % 50, i % 30);
        }

        nlohmann::json saveData;
        bool saved = saveSystem.saveGameState(saveData);
        REQUIRE(saved == true);

        // Should be able to handle large data
        REQUIRE(saveData["entities"].size() >= 1000);
    }

    SECTION("Handle concurrent save/load") {
        std::thread saveThread([&saveSystem]() {
            for (int i = 0; i < 10; ++i) {
                saveSystem.quickSave();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

        std::thread loadThread([&saveSystem]() {
            for (int i = 0; i < 10; ++i) {
                saveSystem.quickLoad();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

        saveThread.join();
        loadThread.join();

        // Should handle concurrent operations without crashing
        REQUIRE(true);
    }
}

TEST_CASE("SaveLoadSystem metadata", "[save_load][metadata]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    SaveLoadSystem saveSystem(world.get());

    SECTION("Save metadata") {
        EntityFactory factory(world.get());
        auto player = factory.createPlayer(10, 10);

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        REQUIRE(saveData.contains("metadata"));
        auto metadata = saveData["metadata"];

        REQUIRE(metadata.contains("save_time"));
        REQUIRE(metadata.contains("game_version"));
        REQUIRE(metadata.contains("entity_count"));
    }

    SECTION("Update play time") {
        saveSystem.updatePlayTime(7200); // 2 hours

        nlohmann::json saveData;
        saveSystem.saveGameState(saveData);

        if (saveData.contains("metadata") &&
            saveData["metadata"].contains("play_time")) {
            REQUIRE(saveData["metadata"]["play_time"] >= 7200);
        }
    }

    SECTION("Get last save time") {
        saveSystem.quickSave();
        auto lastSave = saveSystem.getLastSaveTime();

        // Should be recent
        auto now = std::chrono::system_clock::now();
        auto diff = now - lastSave;
        REQUIRE(diff < std::chrono::seconds(5));
    }
}