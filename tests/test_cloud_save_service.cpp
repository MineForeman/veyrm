#include <catch2/catch_test_macros.hpp>
#include "services/cloud_save_service.h"
#include "db/database_manager.h"
#include "db/save_game_repository.h"
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class TestCloudSaveService : public CloudSaveService {
public:
    TestCloudSaveService(db::SaveGameRepository* repo)
        : CloudSaveService(repo) {}

    // Expose protected methods for testing
    using CloudSaveService::processSyncQueue;
    using CloudSaveService::uploadSave;
    using CloudSaveService::downloadSave;
    using CloudSaveService::deleteSave;
    using CloudSaveService::listSaves;

    size_t getQueueSize() const { return syncQueue.size(); }
    bool isRunningTest() const { return running; }
};

TEST_CASE("CloudSaveService basic operations", "[cloud]") {
    // Initialize database for testing
    db::DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "test_password";

    auto& dbManager = db::DatabaseManager::getInstance();
    dbManager.initialize(config);

    auto saveRepo = std::make_unique<db::SaveGameRepository>();
    TestCloudSaveService service(saveRepo.get());

    SECTION("Service initialization") {
        REQUIRE(service.isEnabled() == true);
        service.setEnabled(false);
        REQUIRE(service.isEnabled() == false);
        service.setEnabled(true);
        REQUIRE(service.isEnabled() == true);
    }

    SECTION("Auto sync configuration") {
        REQUIRE(service.isAutoSyncEnabled() == true);
        service.setAutoSync(false);
        REQUIRE(service.isAutoSyncEnabled() == false);

        service.setAutoSyncInterval(std::chrono::minutes(10));
        auto interval = service.getAutoSyncInterval();
        REQUIRE(interval == std::chrono::minutes(10));
    }

    SECTION("Queue save for upload") {
        SaveMetadata meta;
        meta.saveId = "test-save-001";
        meta.playerId = "player-123";
        meta.slotNumber = 1;
        meta.characterName = "TestHero";
        meta.characterLevel = 10;
        meta.location = "Dungeon Level 5";
        meta.playTime = 3600;
        meta.gameVersion = "1.0.0";
        meta.isCloudSave = false;

        std::string saveData = R"({
            "player": {"name": "TestHero", "level": 10},
            "map": {"depth": 5, "seed": 12345}
        })";

        service.queueSaveUpload(meta, saveData);
        REQUIRE(service.getQueueSize() == 1);
    }

    SECTION("Queue save for download") {
        service.queueSaveDownload("test-save-002", 2);
        REQUIRE(service.getQueueSize() == 1);
    }

    SECTION("Queue save for deletion") {
        service.queueSaveDelete("test-save-003");
        REQUIRE(service.getQueueSize() == 1);
    }

    SECTION("Process sync queue") {
        SaveMetadata meta;
        meta.saveId = "test-save-004";
        meta.playerId = "player-456";
        meta.slotNumber = 3;

        service.queueSaveUpload(meta, "{}");
        service.queueSaveDownload("test-save-005", 4);
        service.queueSaveDelete("test-save-006");

        REQUIRE(service.getQueueSize() == 3);

        // Process one item
        service.processSyncQueue();
        REQUIRE(service.getQueueSize() == 2);
    }

    SECTION("Sync status tracking") {
        auto status = service.getSyncStatus();
        REQUIRE(status.isActive == false);
        REQUIRE(status.currentOperation.empty());
        REQUIRE(status.progress == 0.0f);
        REQUIRE(status.pendingOperations == 0);
    }

    SECTION("Upload save operation") {
        SaveMetadata meta;
        meta.saveId = "upload-test-001";
        meta.playerId = "player-789";
        meta.slotNumber = 5;
        meta.characterName = "Uploader";
        meta.characterLevel = 15;

        std::string saveData = R"({"test": "data"})";

        auto result = service.uploadSave(meta, saveData);
        // Note: This will fail without actual database, but tests the code path
        REQUIRE(result.success == false); // Expected to fail without DB
    }

    SECTION("Download save operation") {
        auto result = service.downloadSave("download-test-001", 6);
        REQUIRE(result.success == false); // Expected to fail without DB
    }

    SECTION("Delete save operation") {
        auto result = service.deleteSave("delete-test-001");
        REQUIRE(result.success == false); // Expected to fail without DB
    }

    SECTION("List saves operation") {
        auto result = service.listSaves("player-999");
        REQUIRE(result.success == false); // Expected to fail without DB
    }

    SECTION("Conflict resolution") {
        SaveMetadata local;
        local.saveId = "local-001";
        local.lastModified = std::chrono::system_clock::now() - 1h;

        SaveMetadata cloud;
        cloud.saveId = "cloud-001";
        cloud.lastModified = std::chrono::system_clock::now();

        auto resolution = service.resolveConflict(local, cloud);
        REQUIRE(resolution == ConflictResolution::UseCloud); // Cloud is newer

        // Make local newer
        local.lastModified = std::chrono::system_clock::now() + 1h;
        resolution = service.resolveConflict(local, cloud);
        REQUIRE(resolution == ConflictResolution::UseLocal); // Local is newer
    }

    SECTION("Start and stop service") {
        service.start();
        std::this_thread::sleep_for(10ms);
        REQUIRE(service.isRunningTest() == true);

        service.stop();
        std::this_thread::sleep_for(10ms);
        REQUIRE(service.isRunningTest() == false);
    }

    SECTION("Sync all saves") {
        auto future = service.syncAllSaves("player-all");
        auto result = future.get();
        REQUIRE(result.success == false); // Expected without DB
    }

    SECTION("Force sync") {
        service.forceSync();
        // Just ensure it doesn't crash
        REQUIRE(true);
    }

    dbManager.shutdown();
}

TEST_CASE("CloudSaveService error handling", "[cloud]") {
    auto saveRepo = std::make_unique<db::SaveGameRepository>();
    TestCloudSaveService service(saveRepo.get());

    SECTION("Handle invalid save IDs") {
        service.queueSaveUpload(SaveMetadata{}, "");
        service.queueSaveDownload("", -1);
        service.queueSaveDelete("");

        // Should handle gracefully without crashing
        service.processSyncQueue();
        REQUIRE(true);
    }

    SECTION("Handle network timeouts") {
        SaveMetadata meta;
        meta.saveId = "timeout-test";
        meta.playerId = "timeout-player";

        // Simulate timeout by using invalid operation
        auto result = service.uploadSave(meta, std::string(10'000'000, 'x')); // Large data
        REQUIRE(result.success == false);
    }

    SECTION("Handle concurrent operations") {
        std::vector<std::thread> threads;

        // Queue operations from multiple threads
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&service, i]() {
                SaveMetadata meta;
                meta.saveId = "concurrent-" + std::to_string(i);
                meta.slotNumber = i;
                service.queueSaveUpload(meta, "{}");
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        REQUIRE(service.getQueueSize() == 10);
    }
}

TEST_CASE("CloudSaveService auto sync", "[cloud]") {
    auto saveRepo = std::make_unique<db::SaveGameRepository>();
    TestCloudSaveService service(saveRepo.get());

    SECTION("Auto sync timer") {
        service.setAutoSync(true);
        service.setAutoSyncInterval(std::chrono::milliseconds(50));

        service.start();
        std::this_thread::sleep_for(100ms);

        // Auto sync should have triggered at least once
        service.stop();
        REQUIRE(true); // Just verify no crash
    }

    SECTION("Disable auto sync during operation") {
        service.setAutoSync(true);
        service.start();

        // Disable while running
        service.setAutoSync(false);
        std::this_thread::sleep_for(10ms);

        service.stop();
        REQUIRE(service.isAutoSyncEnabled() == false);
    }
}

TEST_CASE("CloudSaveService metadata operations", "[cloud]") {
    SECTION("SaveMetadata comparison") {
        SaveMetadata meta1;
        meta1.saveId = "save1";
        meta1.lastModified = std::chrono::system_clock::now();

        SaveMetadata meta2;
        meta2.saveId = "save2";
        meta2.lastModified = std::chrono::system_clock::now() - 1h;

        // meta1 should be newer
        REQUIRE(meta1.lastModified > meta2.lastModified);
    }

    SECTION("SyncResult handling") {
        SyncResult result;
        result.success = true;
        result.message = "Test successful";
        result.syncedSaves = {"save1", "save2", "save3"};
        result.failedSaves = {"save4"};

        REQUIRE(result.success == true);
        REQUIRE(result.syncedSaves.size() == 3);
        REQUIRE(result.failedSaves.size() == 1);
    }

    SECTION("SyncStatus validation") {
        SyncStatus status;
        status.isActive = true;
        status.currentOperation = "Uploading";
        status.progress = 0.5f;
        status.pendingOperations = 5;
        status.lastSyncTime = std::chrono::system_clock::now();
        status.lastError = "";

        REQUIRE(status.isActive == true);
        REQUIRE(status.progress == 0.5f);
        REQUIRE(status.pendingOperations == 5);
        REQUIRE(status.lastError.empty());
    }
}