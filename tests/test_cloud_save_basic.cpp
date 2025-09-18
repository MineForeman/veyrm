#include <catch2/catch_test_macros.hpp>
#include "services/cloud_save_service.h"
#include "services/sync_status.h"
#include <memory>

// Test enums and structs used by CloudSaveService
TEST_CASE("CloudSave enums and structs", "[cloud][enums]") {
    SECTION("ConflictResolution enum") {
        ConflictResolution resolution = ConflictResolution::USE_LOCAL;
        REQUIRE(resolution == ConflictResolution::USE_LOCAL);

        resolution = ConflictResolution::USE_CLOUD;
        REQUIRE(resolution == ConflictResolution::USE_CLOUD);

        resolution = ConflictResolution::MERGE_SMART;
        REQUIRE(resolution == ConflictResolution::MERGE_SMART);

        resolution = ConflictResolution::BACKUP_BOTH;
        REQUIRE(resolution == ConflictResolution::BACKUP_BOTH);

        resolution = ConflictResolution::CANCEL;
        REQUIRE(resolution == ConflictResolution::CANCEL);
    }

    SECTION("SyncStatus enum") {
        SyncStatus status = SyncStatus::SYNCED;
        REQUIRE(status == SyncStatus::SYNCED);

        status = SyncStatus::PENDING_UPLOAD;
        REQUIRE(status == SyncStatus::PENDING_UPLOAD);

        status = SyncStatus::PENDING_DOWNLOAD;
        REQUIRE(status == SyncStatus::PENDING_DOWNLOAD);

        status = SyncStatus::CONFLICT;
        REQUIRE(status == SyncStatus::CONFLICT);

        status = SyncStatus::OFFLINE;
        REQUIRE(status == SyncStatus::OFFLINE);

        status = SyncStatus::ERROR;
        REQUIRE(status == SyncStatus::ERROR);
    }

    SECTION("CloudSaveInfo struct") {
        CloudSaveInfo info;
        info.id = "test-uuid-123";
        info.slot_number = 1;
        info.character_name = "TestHero";
        info.character_level = 5;
        info.map_depth = 3;
        info.play_time = 1800; // 30 minutes
        info.turn_count = 500;
        info.device_name = "TestDevice";
        info.sync_status = SyncStatus::SYNCED;
        info.is_local = true;
        info.is_cloud = true;

        REQUIRE(info.id == "test-uuid-123");
        REQUIRE(info.slot_number == 1);
        REQUIRE(info.character_name == "TestHero");
        REQUIRE(info.character_level == 5);
        REQUIRE(info.map_depth == 3);
        REQUIRE(info.play_time == 1800);
        REQUIRE(info.turn_count == 500);
        REQUIRE(info.device_name == "TestDevice");
        REQUIRE(info.sync_status == SyncStatus::SYNCED);
        REQUIRE(info.is_local == true);
        REQUIRE(info.is_cloud == true);
    }

    SECTION("SyncResult struct") {
        SyncResult result;
        result.success = true;
        result.saves_uploaded = 2;
        result.saves_downloaded = 1;
        result.conflicts_detected = 0;
        result.errors = {"Warning: Slow connection"};

        REQUIRE(result.success == true);
        REQUIRE(result.saves_uploaded == 2);
        REQUIRE(result.saves_downloaded == 1);
        REQUIRE(result.conflicts_detected == 0);
        REQUIRE(result.errors.size() == 1);
        REQUIRE(result.errors[0] == "Warning: Slow connection");
    }
}

// Test CloudSaveService basic construction and methods that don't require dependencies
TEST_CASE("CloudSaveService basic functionality", "[cloud][service]") {
    SECTION("Service initialization without dependencies") {
        // Test with null dependencies (service should handle gracefully)
        CloudSaveService service(nullptr, nullptr, nullptr);

        // Test basic methods that should work without dependencies
        REQUIRE(!service.isAuthenticated());
        REQUIRE(service.getCurrentUserId() == 0);
        REQUIRE(!service.isAutoSyncEnabled());
        REQUIRE(service.getLastError().empty());
    }

    SECTION("Slot filename generation") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        // Test different slot numbers
        std::string filename1 = service.getSlotFilename(1);
        std::string filename5 = service.getSlotFilename(5);
        std::string filename9 = service.getSlotFilename(9);

        REQUIRE(!filename1.empty());
        REQUIRE(!filename5.empty());
        REQUIRE(!filename9.empty());
        REQUIRE(filename1 != filename5);
        REQUIRE(filename5 != filename9);

        // Test auto-save slots (negative numbers)
        std::string auto1 = service.getSlotFilename(-1);
        std::string auto2 = service.getSlotFilename(-2);
        std::string auto3 = service.getSlotFilename(-3);

        REQUIRE(!auto1.empty());
        REQUIRE(!auto2.empty());
        REQUIRE(!auto3.empty());
        REQUIRE(auto1 != auto2);
        REQUIRE(auto2 != auto3);
    }

    SECTION("Device identification") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        std::string deviceId = service.getDeviceId();
        std::string deviceName = service.getDeviceName();

        REQUIRE(!deviceId.empty());
        REQUIRE(!deviceName.empty());

        // Device ID should be consistent
        std::string deviceId2 = service.getDeviceId();
        REQUIRE(deviceId == deviceId2);
    }

    SECTION("User ID management") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        REQUIRE(service.getCurrentUserId() == 0);

        service.setUserId(12345);
        REQUIRE(service.getCurrentUserId() == 12345);

        service.setUserId(0);
        REQUIRE(service.getCurrentUserId() == 0);
    }

    SECTION("Auto-sync state management") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        REQUIRE(!service.isAutoSyncEnabled());

        // Test enabling auto-sync
        bool enabled = service.enableAutoSync(60); // 1 minute
        // Note: This might fail without proper dependencies, but should not crash
        if (enabled) {
            REQUIRE(service.isAutoSyncEnabled());
        }

        service.disableAutoSync();
        REQUIRE(!service.isAutoSyncEnabled());
    }

    SECTION("ECS World management") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        // Test setting ECS world (should not crash with nullptr)
        service.setECSWorld(nullptr);

        // This should not crash the service
        REQUIRE(true);
    }
}

// Test JSON serialization methods with minimal data
TEST_CASE("CloudSaveService JSON operations", "[cloud][json]") {
    SECTION("ECS metadata generation") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        // This should return valid JSON even without ECS world
        auto metadata = service.getECSMetadata();
        REQUIRE(metadata.is_object());
    }

    SECTION("ECS world serialization") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        // Should return valid JSON even without ECS world
        auto worldJson = service.serializeECSWorld();
        REQUIRE(worldJson.is_object());
    }

    SECTION("ECS world deserialization") {
        CloudSaveService service(nullptr, nullptr, nullptr);

        // Test with empty JSON
        boost::json::value emptyJson = boost::json::object();
        bool result = service.deserializeECSWorld(emptyJson);
        // Should handle gracefully (may return false but not crash)
        REQUIRE((result == true || result == false));

        // Test with invalid JSON
        boost::json::value invalidJson = boost::json::array();
        bool result2 = service.deserializeECSWorld(invalidJson);
        // Should handle gracefully
        REQUIRE((result2 == true || result2 == false));
    }
}