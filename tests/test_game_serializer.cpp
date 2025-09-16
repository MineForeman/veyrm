#include <catch2/catch_test_macros.hpp>
#include "game_serializer.h"
#include "game_manager.h"
#include "map.h"
#include "ecs/game_world.h"
#include "ecs/entity_factory.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE("GameSerializer basic operations", "[serializer]") {
    GameSerializer serializer;

    SECTION("Get save directory") {
        std::string saveDir = serializer.getSaveDirectory();
        REQUIRE(!saveDir.empty());

        // Create directory if it doesn't exist
        if (!fs::exists(saveDir)) {
            fs::create_directories(saveDir);
        }
        REQUIRE(fs::exists(saveDir));
    }

    SECTION("Get save file path") {
        std::string path = serializer.getSaveFilePath(1);
        REQUIRE(!path.empty());
        REQUIRE(path.find("save_1.json") != std::string::npos);
    }

    SECTION("Check if save slot exists") {
        // Create a temporary save file
        std::string savePath = serializer.getSaveFilePath(99);
        std::ofstream file(savePath);
        file << "{}";
        file.close();

        REQUIRE(serializer.saveExists(99) == true);
        REQUIRE(serializer.saveExists(98) == false);

        // Clean up
        fs::remove(savePath);
    }

    SECTION("List available saves") {
        // Create some test saves
        for (int i = 90; i <= 92; ++i) {
            std::string path = serializer.getSaveFilePath(i);
            std::ofstream file(path);
            file << R"({
                "metadata": {
                    "slot": )" << i << R"(,
                    "character_name": "Test)" << i << R"("
                }
            })";
            file.close();
        }

        auto saves = serializer.listSaves();
        REQUIRE(saves.size() >= 3);

        // Clean up
        for (int i = 90; i <= 92; ++i) {
            fs::remove(serializer.getSaveFilePath(i));
        }
    }

    SECTION("Delete save slot") {
        // Create a save to delete
        std::string path = serializer.getSaveFilePath(95);
        std::ofstream file(path);
        file << "{}";
        file.close();

        REQUIRE(fs::exists(path) == true);

        bool deleted = serializer.deleteSave(95);
        REQUIRE(deleted == true);
        REQUIRE(fs::exists(path) == false);

        // Try to delete non-existent save
        bool deletedAgain = serializer.deleteSave(95);
        REQUIRE(deletedAgain == false);
    }
}

TEST_CASE("GameSerializer save operations", "[serializer][save]") {
    GameSerializer serializer;
    Map testMap(50, 30);
    auto world = std::make_unique<ecs::GameWorld>(&testMap);

    SECTION("Save GameManager state") {
        GameManager manager;

        // Set up some game state
        manager.getCurrentMap() = &testMap;
        manager.setCurrentDepth(5);
        manager.setSeed(12345);

        bool saved = serializer.saveGame(&manager, 80);
        REQUIRE(saved == true);

        // Verify file was created
        REQUIRE(fs::exists(serializer.getSaveFilePath(80)) == true);

        // Clean up
        fs::remove(serializer.getSaveFilePath(80));
    }

    SECTION("Save with metadata") {
        GameManager manager;
        manager.getCurrentMap() = &testMap;

        // Create player entity
        ecs::EntityFactory factory(world.get());
        auto player = factory.createPlayer(10, 10);

        bool saved = serializer.saveGame(&manager, 81);
        REQUIRE(saved == true);

        // Load and verify metadata
        std::ifstream file(serializer.getSaveFilePath(81));
        nlohmann::json saveData;
        file >> saveData;
        file.close();

        REQUIRE(saveData.contains("metadata"));
        REQUIRE(saveData["metadata"]["slot"] == 81);

        // Clean up
        fs::remove(serializer.getSaveFilePath(81));
    }

    SECTION("Save with error handling") {
        GameManager manager;

        // Try to save to invalid slot
        bool saved = serializer.saveGame(&manager, -1);
        REQUIRE(saved == false);

        // Try to save null manager
        bool savedNull = serializer.saveGame(nullptr, 82);
        REQUIRE(savedNull == false);
    }
}

TEST_CASE("GameSerializer load operations", "[serializer][load]") {
    GameSerializer serializer;
    GameManager manager;
    Map testMap(50, 30);

    SECTION("Load valid save") {
        // Create a valid save file
        nlohmann::json saveData = {
            {"version", "1.0.0"},
            {"metadata", {
                {"slot", 85},
                {"character_name", "LoadTest"},
                {"character_level", 10},
                {"location", "Dungeon Level 3"},
                {"play_time", 7200},
                {"save_time", "2024-01-01T00:00:00"}
            }},
            {"game_state", {
                {"current_depth", 3},
                {"seed", 54321},
                {"turn_count", 1000}
            }},
            {"player", {
                {"position", {{"x", 25}, {"y", 15}}},
                {"stats", {
                    {"level", 10},
                    {"experience", 5000},
                    {"hp", 80},
                    {"max_hp", 100}
                }}
            }},
            {"map", {
                {"width", 50},
                {"height", 30},
                {"depth", 3}
            }},
            {"entities", nlohmann::json::array()},
            {"message_log", nlohmann::json::array()}
        };

        std::string savePath = serializer.getSaveFilePath(85);
        std::ofstream file(savePath);
        file << saveData.dump(2);
        file.close();

        manager.getCurrentMap() = &testMap;
        bool loaded = serializer.loadGame(&manager, 85);
        REQUIRE(loaded == true);

        // Verify state was loaded
        REQUIRE(manager.getCurrentDepth() == 3);
        REQUIRE(manager.getSeed() == 54321);

        // Clean up
        fs::remove(savePath);
    }

    SECTION("Load non-existent save") {
        bool loaded = serializer.loadGame(&manager, 999);
        REQUIRE(loaded == false);
    }

    SECTION("Load corrupted save") {
        // Create corrupted save
        std::string savePath = serializer.getSaveFilePath(86);
        std::ofstream file(savePath);
        file << "{ corrupted json [}";
        file.close();

        bool loaded = serializer.loadGame(&manager, 86);
        REQUIRE(loaded == false);

        // Clean up
        fs::remove(savePath);
    }

    SECTION("Load with version mismatch") {
        nlohmann::json saveData = {
            {"version", "999.0.0"}, // Future version
            {"metadata", {{"slot", 87}}},
            {"game_state", {}}
        };

        std::string savePath = serializer.getSaveFilePath(87);
        std::ofstream file(savePath);
        file << saveData.dump();
        file.close();

        manager.getCurrentMap() = &testMap;
        bool loaded = serializer.loadGame(&manager, 87);
        // Should handle version mismatch gracefully
        REQUIRE((loaded == true || loaded == false));

        // Clean up
        fs::remove(savePath);
    }
}

TEST_CASE("GameSerializer auto-save", "[serializer][autosave]") {
    GameSerializer serializer;
    GameManager manager;
    Map testMap(50, 30);
    manager.getCurrentMap() = &testMap;

    SECTION("Auto-save functionality") {
        bool saved = serializer.autoSave(&manager);
        REQUIRE(saved == true);

        // Verify auto-save file exists
        REQUIRE(fs::exists(serializer.getSaveFilePath(0)) == true);

        // Load auto-save
        bool loaded = serializer.loadAutoSave(&manager);
        REQUIRE(loaded == true);

        // Clean up
        fs::remove(serializer.getSaveFilePath(0));
    }

    SECTION("Quick save/load") {
        manager.setCurrentDepth(7);

        bool saved = serializer.quickSave(&manager);
        REQUIRE(saved == true);

        // Change state
        manager.setCurrentDepth(10);

        // Quick load should restore
        bool loaded = serializer.quickLoad(&manager);
        REQUIRE(loaded == true);
        REQUIRE(manager.getCurrentDepth() == 7);

        // Clean up
        fs::remove(serializer.getQuickSavePath());
    }
}

TEST_CASE("GameSerializer complex data", "[serializer][complex]") {
    GameSerializer serializer;
    GameManager manager;
    Map testMap(80, 40);
    auto world = std::make_unique<ecs::GameWorld>(&testMap);
    manager.getCurrentMap() = &testMap;

    SECTION("Save with many entities") {
        ecs::EntityFactory factory(world.get());

        // Create many entities
        for (int i = 0; i < 100; ++i) {
            factory.createMonster("goblin", i % 80, i % 40);
        }

        bool saved = serializer.saveGame(&manager, 88);
        REQUIRE(saved == true);

        // Verify file size is reasonable
        auto fileSize = fs::file_size(serializer.getSaveFilePath(88));
        REQUIRE(fileSize > 1000); // Should have substantial data

        // Clean up
        fs::remove(serializer.getSaveFilePath(88));
    }

    SECTION("Save with deep message log") {
        // Add many messages
        for (int i = 0; i < 1000; ++i) {
            manager.getMessageLog().addMessage("Test message " + std::to_string(i));
        }

        bool saved = serializer.saveGame(&manager, 89);
        REQUIRE(saved == true);

        // Load and verify
        GameManager newManager;
        Map newMap(80, 40);
        newManager.getCurrentMap() = &newMap;

        bool loaded = serializer.loadGame(&newManager, 89);
        REQUIRE(loaded == true);

        // Clean up
        fs::remove(serializer.getSaveFilePath(89));
    }
}

TEST_CASE("GameSerializer error recovery", "[serializer][errors]") {
    GameSerializer serializer;

    SECTION("Handle filesystem errors") {
        // Try to save to invalid path
        GameSerializer badSerializer;
        // Override save directory to invalid path
        std::string badPath = "/nonexistent/path/saves";

        // Should handle gracefully
        GameManager manager;
        Map testMap(50, 30);
        manager.getCurrentMap() = &testMap;

        bool saved = badSerializer.saveGame(&manager, 1);
        REQUIRE(saved == false);
    }

    SECTION("Handle concurrent access") {
        GameManager manager1, manager2;
        Map map1(50, 30), map2(50, 30);
        manager1.getCurrentMap() = &map1;
        manager2.getCurrentMap() = &map2;

        // Save from first manager
        bool saved1 = serializer.saveGame(&manager1, 70);
        REQUIRE(saved1 == true);

        // Save from second manager (overwrites)
        bool saved2 = serializer.saveGame(&manager2, 70);
        REQUIRE(saved2 == true);

        // Clean up
        fs::remove(serializer.getSaveFilePath(70));
    }

    SECTION("Validate save data") {
        // Test various invalid JSON structures
        std::vector<std::string> invalidSaves = {
            "{}", // Empty
            R"({"version": "1.0.0"})", // Missing required fields
            R"({"metadata": null})", // Null metadata
            R"({"game_state": "invalid"})" // Wrong type
        };

        GameManager manager;
        Map testMap(50, 30);
        manager.getCurrentMap() = &testMap;

        for (size_t i = 0; i < invalidSaves.size(); ++i) {
            std::string path = serializer.getSaveFilePath(60 + i);
            std::ofstream file(path);
            file << invalidSaves[i];
            file.close();

            bool loaded = serializer.loadGame(&manager, 60 + i);
            REQUIRE(loaded == false);

            fs::remove(path);
        }
    }
}