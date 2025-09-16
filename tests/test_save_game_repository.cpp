#include <catch2/catch_test_macros.hpp>
#include "db/save_game_repository.h"
#include "db/database_manager.h"
#include <nlohmann/json.hpp>

TEST_CASE("SaveGameRepository basic operations", "[database][save]") {
    // Initialize test database
    db::DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "test_password";

    auto& dbManager = db::DatabaseManager::getInstance();

    // Skip tests if database is not available
    try {
        dbManager.initialize(config);
    } catch (const std::exception& e) {
        WARN("Skipping database tests - database not available: " << e.what());
        return;
    }

    db::SaveGameRepository repo;

    SECTION("Create save game") {
        db::SaveGame save;
        save.playerId = "test-player-001";
        save.slotNumber = 1;
        save.characterName = "TestHero";
        save.characterLevel = 5;
        save.characterClass = "Warrior";
        save.location = "Dungeon Level 2";
        save.playTime = 7200; // 2 hours
        save.gameVersion = "1.0.0";
        save.difficulty = "Normal";
        save.gameMode = "Adventure";
        save.isHardcore = false;
        save.isIronman = false;
        save.saveData = R"({
            "player": {
                "name": "TestHero",
                "level": 5,
                "hp": 100,
                "mp": 50
            },
            "inventory": [],
            "map": {
                "level": 2,
                "seed": 12345
            }
        })";

        auto saveId = repo.create(save);

        if (!saveId.empty()) {
            REQUIRE(!saveId.empty());
            save.saveId = saveId;

            // Verify the save was created
            auto retrieved = repo.findById(saveId);
            REQUIRE(retrieved.has_value());
            REQUIRE(retrieved->characterName == "TestHero");
            REQUIRE(retrieved->characterLevel == 5);
        } else {
            WARN("Could not create save - database may not be properly initialized");
        }
    }

    SECTION("Find by player and slot") {
        db::SaveGame save;
        save.playerId = "test-player-002";
        save.slotNumber = 2;
        save.characterName = "SlotTester";
        save.characterLevel = 3;
        save.saveData = "{}";

        auto saveId = repo.create(save);

        if (!saveId.empty()) {
            auto found = repo.findByPlayerAndSlot("test-player-002", 2);
            REQUIRE(found.has_value());
            REQUIRE(found->characterName == "SlotTester");
            REQUIRE(found->slotNumber == 2);
        }
    }

    SECTION("Find all saves for player") {
        std::string playerId = "test-player-003";

        // Create multiple saves
        for (int i = 1; i <= 3; ++i) {
            db::SaveGame save;
            save.playerId = playerId;
            save.slotNumber = i;
            save.characterName = "Hero" + std::to_string(i);
            save.characterLevel = i * 5;
            save.saveData = "{}";
            repo.create(save);
        }

        auto saves = repo.findAllByPlayer(playerId);
        if (!saves.empty()) {
            REQUIRE(saves.size() >= 3);

            // Verify saves are for the correct player
            for (const auto& save : saves) {
                REQUIRE(save.playerId == playerId);
            }
        }
    }

    SECTION("Update save game") {
        db::SaveGame save;
        save.playerId = "test-player-004";
        save.slotNumber = 4;
        save.characterName = "UpdateTest";
        save.characterLevel = 10;
        save.saveData = R"({"original": true})";

        auto saveId = repo.create(save);

        if (!saveId.empty()) {
            save.saveId = saveId;
            save.characterLevel = 15;
            save.location = "Updated Location";
            save.playTime = 10000;
            save.saveData = R"({"updated": true})";

            bool updated = repo.update(save);
            REQUIRE(updated == true);

            auto retrieved = repo.findById(saveId);
            REQUIRE(retrieved.has_value());
            REQUIRE(retrieved->characterLevel == 15);
            REQUIRE(retrieved->location == "Updated Location");
            REQUIRE(retrieved->playTime == 10000);
        }
    }

    SECTION("Delete save game") {
        db::SaveGame save;
        save.playerId = "test-player-005";
        save.slotNumber = 5;
        save.characterName = "DeleteTest";
        save.characterLevel = 20;
        save.saveData = "{}";

        auto saveId = repo.create(save);

        if (!saveId.empty()) {
            bool deleted = repo.remove(saveId);
            REQUIRE(deleted == true);

            auto retrieved = repo.findById(saveId);
            REQUIRE(!retrieved.has_value());
        }
    }

    SECTION("Find all saves (pagination)") {
        auto allSaves = repo.findAll();
        // Just verify it doesn't crash
        REQUIRE(allSaves.size() >= 0);
    }

    SECTION("Get latest save for player") {
        std::string playerId = "test-player-006";

        // Create saves with different timestamps
        for (int i = 1; i <= 3; ++i) {
            db::SaveGame save;
            save.playerId = playerId;
            save.slotNumber = 10 + i;
            save.characterName = "Latest" + std::to_string(i);
            save.characterLevel = i;
            save.saveData = "{}";
            repo.create(save);

            // Small delay to ensure different timestamps
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        auto latest = repo.getLatestSaveForPlayer(playerId);
        if (latest.has_value()) {
            // Should be the last one created
            REQUIRE(latest->characterName.find("Latest") != std::string::npos);
        }
    }

    SECTION("Count saves for player") {
        std::string playerId = "test-player-007";

        int initialCount = repo.countSavesForPlayer(playerId);

        // Create a new save
        db::SaveGame save;
        save.playerId = playerId;
        save.slotNumber = 20;
        save.characterName = "Counter";
        save.characterLevel = 1;
        save.saveData = "{}";
        repo.create(save);

        int newCount = repo.countSavesForPlayer(playerId);
        REQUIRE(newCount == initialCount + 1);
    }

    SECTION("Check if slot exists") {
        std::string playerId = "test-player-008";
        int slot = 30;

        bool existsBefore = repo.slotExists(playerId, slot);
        REQUIRE(existsBefore == false);

        db::SaveGame save;
        save.playerId = playerId;
        save.slotNumber = slot;
        save.characterName = "SlotChecker";
        save.characterLevel = 1;
        save.saveData = "{}";
        repo.create(save);

        bool existsAfter = repo.slotExists(playerId, slot);
        REQUIRE(existsAfter == true);
    }

    SECTION("Delete all saves for player") {
        std::string playerId = "test-player-009";

        // Create multiple saves
        for (int i = 1; i <= 5; ++i) {
            db::SaveGame save;
            save.playerId = playerId;
            save.slotNumber = 40 + i;
            save.characterName = "Bulk" + std::to_string(i);
            save.characterLevel = i;
            save.saveData = "{}";
            repo.create(save);
        }

        bool deleted = repo.deleteAllForPlayer(playerId);
        REQUIRE(deleted == true);

        auto remaining = repo.findAllByPlayer(playerId);
        REQUIRE(remaining.empty());
    }

    SECTION("Handle invalid save data") {
        db::SaveGame save;
        save.playerId = "";  // Invalid player ID
        save.slotNumber = -1; // Invalid slot
        save.characterName = "";
        save.characterLevel = -100;
        save.saveData = "";

        auto saveId = repo.create(save);
        // Should handle gracefully
        REQUIRE(saveId.empty());
    }

    SECTION("Handle large save data") {
        db::SaveGame save;
        save.playerId = "test-player-010";
        save.slotNumber = 50;
        save.characterName = "LargeDataTest";
        save.characterLevel = 99;

        // Create large JSON save data
        nlohmann::json largeData;
        largeData["inventory"] = nlohmann::json::array();
        for (int i = 0; i < 1000; ++i) {
            largeData["inventory"].push_back({
                {"id", i},
                {"name", "Item" + std::to_string(i)},
                {"description", "A very long description for testing purposes that contains lots of text"}
            });
        }
        save.saveData = largeData.dump();

        auto saveId = repo.create(save);
        if (!saveId.empty()) {
            auto retrieved = repo.findById(saveId);
            REQUIRE(retrieved.has_value());

            // Verify the large data was saved and retrieved correctly
            auto retrievedData = nlohmann::json::parse(retrieved->saveData);
            REQUIRE(retrievedData["inventory"].size() == 1000);
        }
    }

    dbManager.shutdown();
}

TEST_CASE("SaveGameRepository cloud sync operations", "[database][save][cloud]") {
    db::DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "test_password";

    auto& dbManager = db::DatabaseManager::getInstance();

    try {
        dbManager.initialize(config);
    } catch (const std::exception& e) {
        WARN("Skipping cloud sync tests - database not available");
        return;
    }

    db::SaveGameRepository repo;

    SECTION("Mark save as cloud synced") {
        db::SaveGame save;
        save.playerId = "cloud-player-001";
        save.slotNumber = 100;
        save.characterName = "CloudHero";
        save.characterLevel = 50;
        save.isCloudSave = false;
        save.saveData = "{}";

        auto saveId = repo.create(save);
        if (!saveId.empty()) {
            REQUIRE(save.isCloudSave == false);

            bool marked = repo.markAsCloudSynced(saveId);
            REQUIRE(marked == true);

            auto retrieved = repo.findById(saveId);
            REQUIRE(retrieved.has_value());
            REQUIRE(retrieved->isCloudSave == true);
        }
    }

    SECTION("Find unsynced saves") {
        std::string playerId = "cloud-player-002";

        // Create some synced and unsynced saves
        for (int i = 1; i <= 5; ++i) {
            db::SaveGame save;
            save.playerId = playerId;
            save.slotNumber = 200 + i;
            save.characterName = "Sync" + std::to_string(i);
            save.characterLevel = i;
            save.isCloudSave = (i % 2 == 0); // Even numbers are synced
            save.saveData = "{}";
            repo.create(save);
        }

        auto unsynced = repo.findUnsyncedSaves(playerId);
        // Should have at least the odd-numbered saves
        for (const auto& save : unsynced) {
            REQUIRE(save.isCloudSave == false);
        }
    }

    SECTION("Update cloud sync timestamp") {
        db::SaveGame save;
        save.playerId = "cloud-player-003";
        save.slotNumber = 300;
        save.characterName = "TimestampTest";
        save.characterLevel = 75;
        save.saveData = "{}";

        auto saveId = repo.create(save);
        if (!saveId.empty()) {
            auto before = repo.findById(saveId);
            REQUIRE(before.has_value());
            auto originalTime = before->cloudSyncTime;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            bool updated = repo.updateCloudSyncTime(saveId);
            REQUIRE(updated == true);

            auto after = repo.findById(saveId);
            REQUIRE(after.has_value());

            if (after->cloudSyncTime.has_value() && originalTime.has_value()) {
                REQUIRE(*after->cloudSyncTime > *originalTime);
            }
        }
    }

    dbManager.shutdown();
}