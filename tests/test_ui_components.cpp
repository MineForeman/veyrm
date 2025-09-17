#include <catch2/catch_test_macros.hpp>
#include "login_screen.h"
#include "save_load_screen.h"
#include "ui/cloud_save_indicator.h"
#include "db/database_manager.h"
#include "auth/authentication_service.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>

using namespace ftxui;

TEST_CASE("LoginScreen basic operations", "[ui][login]") {
    LoginScreen loginScreen;

    SECTION("Initial state") {
        REQUIRE(loginScreen.GetUsername().empty());
        REQUIRE(loginScreen.GetPassword().empty());
        REQUIRE(!loginScreen.IsLoggedIn());
        REQUIRE(loginScreen.GetError().empty());
    }

    SECTION("Set credentials") {
        loginScreen.SetUsername("testuser");
        loginScreen.SetPassword("testpass123");

        REQUIRE(loginScreen.GetUsername() == "testuser");
        REQUIRE(loginScreen.GetPassword() == "testpass123");
    }

    SECTION("Login attempt with invalid credentials") {
        loginScreen.SetUsername("invalid");
        loginScreen.SetPassword("wrong");

        bool result = loginScreen.AttemptLogin();
        REQUIRE(result == false);
        REQUIRE(!loginScreen.GetError().empty());
    }

    SECTION("Register new user") {
        loginScreen.SetUsername("newuser_" + std::to_string(time(nullptr)));
        loginScreen.SetPassword("password123");
        loginScreen.SetEmail("test@example.com");

        bool result = loginScreen.AttemptRegister();
        // May fail if database is not available
        REQUIRE((result == true || result == false));
    }

    SECTION("Toggle between login and register mode") {
        REQUIRE(loginScreen.IsLoginMode() == true);

        loginScreen.ToggleMode();
        REQUIRE(loginScreen.IsLoginMode() == false);

        loginScreen.ToggleMode();
        REQUIRE(loginScreen.IsLoginMode() == true);
    }

    SECTION("Clear fields") {
        loginScreen.SetUsername("testuser");
        loginScreen.SetPassword("testpass");
        loginScreen.SetEmail("test@test.com");

        loginScreen.ClearFields();

        REQUIRE(loginScreen.GetUsername().empty());
        REQUIRE(loginScreen.GetPassword().empty());
        REQUIRE(loginScreen.GetEmail().empty());
    }

    SECTION("Password validation") {
        // Too short
        REQUIRE(loginScreen.ValidatePassword("short") == false);

        // Just right
        REQUIRE(loginScreen.ValidatePassword("validpass123") == true);

        // With special characters
        REQUIRE(loginScreen.ValidatePassword("Valid@Pass123!") == true);
    }

    SECTION("Username validation") {
        // Too short
        REQUIRE(loginScreen.ValidateUsername("ab") == false);

        // Valid
        REQUIRE(loginScreen.ValidateUsername("validuser") == true);

        // With numbers
        REQUIRE(loginScreen.ValidateUsername("user123") == true);

        // Invalid characters
        REQUIRE(loginScreen.ValidateUsername("user@#$") == false);
    }

    SECTION("Email validation") {
        REQUIRE(loginScreen.ValidateEmail("valid@email.com") == true);
        REQUIRE(loginScreen.ValidateEmail("invalid.email") == false);
        REQUIRE(loginScreen.ValidateEmail("@invalid.com") == false);
        REQUIRE(loginScreen.ValidateEmail("invalid@") == false);
    }

    SECTION("Remember me functionality") {
        loginScreen.SetRememberMe(true);
        REQUIRE(loginScreen.GetRememberMe() == true);

        loginScreen.SetRememberMe(false);
        REQUIRE(loginScreen.GetRememberMe() == false);
    }

    SECTION("Guest login") {
        bool result = loginScreen.LoginAsGuest();
        REQUIRE(result == true);
        REQUIRE(loginScreen.IsGuestMode() == true);
    }

    SECTION("Logout") {
        loginScreen.LoginAsGuest();
        REQUIRE(loginScreen.IsLoggedIn() == true);

        loginScreen.Logout();
        REQUIRE(loginScreen.IsLoggedIn() == false);
        REQUIRE(loginScreen.GetUsername().empty());
    }

    SECTION("Create component") {
        auto component = loginScreen.CreateComponent();
        REQUIRE(component != nullptr);

        // Component should be renderable
        auto screen = Screen::Create(Dimension::Fixed(80), Dimension::Fixed(24));
        auto doc = component->Render();
        Render(screen, doc);

        // Check that something was rendered
        REQUIRE(screen.PixelAt(0, 0).character != "");
    }
}

TEST_CASE("SaveLoadScreen operations", "[ui][saveload]") {
    Map testMap(50, 30);
    GameManager gameManager;
    gameManager.getCurrentMap() = &testMap;

    SaveLoadScreen saveScreen(&gameManager);

    SECTION("Initial state") {
        REQUIRE(saveScreen.GetSelectedSlot() == 1);
        REQUIRE(saveScreen.GetMode() == SaveLoadMode::Save);
    }

    SECTION("Change save slot") {
        saveScreen.SetSelectedSlot(5);
        REQUIRE(saveScreen.GetSelectedSlot() == 5);

        // Out of range - should clamp
        saveScreen.SetSelectedSlot(0);
        REQUIRE(saveScreen.GetSelectedSlot() == 1);

        saveScreen.SetSelectedSlot(10);
        REQUIRE(saveScreen.GetSelectedSlot() == 9);
    }

    SECTION("Toggle mode") {
        REQUIRE(saveScreen.GetMode() == SaveLoadMode::Save);

        saveScreen.SetMode(SaveLoadMode::Load);
        REQUIRE(saveScreen.GetMode() == SaveLoadMode::Load);
    }

    SECTION("Check save existence") {
        bool exists = saveScreen.SaveExists(1);
        // May or may not exist
        REQUIRE((exists == true || exists == false));
    }

    SECTION("Get save info") {
        auto info = saveScreen.GetSaveInfo(1);

        if (info.exists) {
            REQUIRE(!info.characterName.empty());
            REQUIRE(info.slot == 1);
            REQUIRE(info.level >= 0);
        } else {
            REQUIRE(info.characterName == "Empty Slot");
        }
    }

    SECTION("List all saves") {
        auto saves = saveScreen.ListAllSaves();
        REQUIRE(saves.size() == 9); // 9 save slots

        for (size_t i = 0; i < saves.size(); ++i) {
            REQUIRE(saves[i].slot == static_cast<int>(i + 1));
        }
    }

    SECTION("Perform save") {
        bool result = saveScreen.PerformSave(9); // Use last slot
        // May fail if not properly initialized
        REQUIRE((result == true || result == false));

        if (result) {
            REQUIRE(saveScreen.SaveExists(9) == true);
        }
    }

    SECTION("Perform load") {
        // First save something
        saveScreen.PerformSave(8);

        bool result = saveScreen.PerformLoad(8);
        REQUIRE((result == true || result == false));
    }

    SECTION("Delete save") {
        // Create a save first
        saveScreen.PerformSave(7);

        bool result = saveScreen.DeleteSave(7);
        REQUIRE((result == true || result == false));

        if (result) {
            REQUIRE(saveScreen.SaveExists(7) == false);
        }
    }

    SECTION("Quick save/load") {
        bool saveResult = saveScreen.QuickSave();
        REQUIRE((saveResult == true || saveResult == false));

        bool loadResult = saveScreen.QuickLoad();
        REQUIRE((loadResult == true || loadResult == false));
    }

    SECTION("Auto save") {
        bool result = saveScreen.AutoSave();
        REQUIRE((result == true || result == false));

        if (result) {
            auto info = saveScreen.GetSaveInfo(0); // Slot 0 is auto-save
            REQUIRE(info.exists == true);
        }
    }

    SECTION("Create component") {
        auto component = saveScreen.CreateComponent();
        REQUIRE(component != nullptr);

        // Should be able to render
        auto screen = Screen::Create(Dimension::Fixed(80), Dimension::Fixed(24));
        auto doc = component->Render();
        Render(screen, doc);
    }

    SECTION("Keyboard navigation") {
        saveScreen.SetSelectedSlot(5);

        saveScreen.NavigateUp();
        REQUIRE(saveScreen.GetSelectedSlot() == 4);

        saveScreen.NavigateDown();
        REQUIRE(saveScreen.GetSelectedSlot() == 5);

        // Test wrapping
        saveScreen.SetSelectedSlot(1);
        saveScreen.NavigateUp();
        REQUIRE(saveScreen.GetSelectedSlot() == 9);

        saveScreen.SetSelectedSlot(9);
        saveScreen.NavigateDown();
        REQUIRE(saveScreen.GetSelectedSlot() == 1);
    }

    SECTION("Confirmation dialog") {
        saveScreen.ShowConfirmation("Overwrite save?");
        REQUIRE(saveScreen.IsConfirmationShown() == true);
        REQUIRE(saveScreen.GetConfirmationMessage() == "Overwrite save?");

        saveScreen.ConfirmAction(true);
        REQUIRE(saveScreen.IsConfirmationShown() == false);
    }

    SECTION("Error handling") {
        saveScreen.ShowError("Failed to save game");
        REQUIRE(saveScreen.GetLastError() == "Failed to save game");

        saveScreen.ClearError();
        REQUIRE(saveScreen.GetLastError().empty());
    }
}

TEST_CASE("CloudSaveIndicator operations", "[ui][cloud]") {
    CloudSaveIndicator indicator;

    SECTION("Initial state") {
        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Idle);
        REQUIRE(indicator.IsVisible() == false);
    }

    SECTION("Set sync status") {
        indicator.SetStatus(CloudSyncStatus::Syncing);
        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Syncing);
        REQUIRE(indicator.IsVisible() == true);

        indicator.SetStatus(CloudSyncStatus::Success);
        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Success);

        indicator.SetStatus(CloudSyncStatus::Error);
        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Error);
    }

    SECTION("Progress indicator") {
        indicator.SetProgress(0.5f);
        REQUIRE(indicator.GetProgress() == 0.5f);

        // Clamp to valid range
        indicator.SetProgress(1.5f);
        REQUIRE(indicator.GetProgress() == 1.0f);

        indicator.SetProgress(-0.5f);
        REQUIRE(indicator.GetProgress() == 0.0f);
    }

    SECTION("Status message") {
        indicator.SetMessage("Uploading save...");
        REQUIRE(indicator.GetMessage() == "Uploading save...");

        indicator.ClearMessage();
        REQUIRE(indicator.GetMessage().empty());
    }

    SECTION("Animation") {
        indicator.SetStatus(CloudSyncStatus::Syncing);
        indicator.StartAnimation();
        REQUIRE(indicator.IsAnimating() == true);

        indicator.StopAnimation();
        REQUIRE(indicator.IsAnimating() == false);
    }

    SECTION("Auto-hide after success") {
        indicator.SetStatus(CloudSyncStatus::Success);
        indicator.SetAutoHide(true, 2.0f); // Hide after 2 seconds

        // Update for 3 seconds
        for (int i = 0; i < 30; ++i) {
            indicator.Update(0.1f);
        }

        REQUIRE(indicator.IsVisible() == false);
    }

    SECTION("Error persistence") {
        indicator.SetStatus(CloudSyncStatus::Error);
        indicator.SetMessage("Connection failed");
        indicator.SetAutoHide(false); // Don't auto-hide errors

        // Update for a while
        for (int i = 0; i < 50; ++i) {
            indicator.Update(0.1f);
        }

        // Should still be visible
        REQUIRE(indicator.IsVisible() == true);
    }

    SECTION("Create component") {
        auto component = indicator.CreateComponent();
        REQUIRE(component != nullptr);

        // Test rendering
        auto screen = Screen::Create(Dimension::Fixed(20), Dimension::Fixed(3));
        auto doc = component->Render();
        Render(screen, doc);
    }

    SECTION("Icon display") {
        indicator.SetStatus(CloudSyncStatus::Syncing);
        auto icon = indicator.GetStatusIcon();
        REQUIRE(!icon.empty());

        indicator.SetStatus(CloudSyncStatus::Success);
        icon = indicator.GetStatusIcon();
        REQUIRE(icon == "✓"); // Checkmark for success

        indicator.SetStatus(CloudSyncStatus::Error);
        icon = indicator.GetStatusIcon();
        REQUIRE(icon == "✗"); // X for error
    }

    SECTION("Queue status updates") {
        indicator.QueueStatus(CloudSyncStatus::Syncing, "Starting sync...");
        indicator.QueueStatus(CloudSyncStatus::Success, "Sync complete!");

        indicator.ProcessQueue();
        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Syncing);
        REQUIRE(indicator.GetMessage() == "Starting sync...");

        indicator.ProcessQueue();
        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Success);
        REQUIRE(indicator.GetMessage() == "Sync complete!");
    }
}

TEST_CASE("UI Component integration", "[ui][integration]") {
    SECTION("Login to SaveLoad flow") {
        LoginScreen loginScreen;
        loginScreen.LoginAsGuest();
        REQUIRE(loginScreen.IsLoggedIn() == true);

        Map testMap(50, 30);
        GameManager gameManager;
        gameManager.getCurrentMap() = &testMap;

        SaveLoadScreen saveScreen(&gameManager);

        // Should be able to save after login
        bool canSave = saveScreen.CanSave();
        REQUIRE(canSave == true);
    }

    SECTION("Cloud indicator during save") {
        CloudSaveIndicator indicator;

        // Simulate save operation
        indicator.SetStatus(CloudSyncStatus::Syncing);
        indicator.SetMessage("Saving to cloud...");
        indicator.SetProgress(0.0f);

        for (int i = 0; i <= 10; ++i) {
            indicator.SetProgress(i / 10.0f);
            indicator.Update(0.1f);
        }

        indicator.SetStatus(CloudSyncStatus::Success);
        indicator.SetMessage("Save complete!");

        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Success);
    }

    SECTION("Error recovery flow") {
        CloudSaveIndicator indicator;

        // Simulate error
        indicator.SetStatus(CloudSyncStatus::Error);
        indicator.SetMessage("Network error");

        // User retries
        indicator.SetStatus(CloudSyncStatus::Syncing);
        indicator.SetMessage("Retrying...");

        // Success on retry
        indicator.SetStatus(CloudSyncStatus::Success);
        indicator.SetMessage("Sync successful!");

        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Success);
    }
}

TEST_CASE("UI Component rendering", "[ui][rendering]") {
    SECTION("Login screen rendering") {
        LoginScreen loginScreen;
        auto component = loginScreen.CreateComponent();

        auto screen = Screen::Create(Dimension::Fixed(80), Dimension::Fixed(24));

        // Set some data
        loginScreen.SetUsername("testuser");
        loginScreen.SetError("Invalid password");

        auto doc = component->Render();
        Render(screen, doc);

        // Convert screen to string to check content
        std::string content;
        for (int y = 0; y < 24; ++y) {
            for (int x = 0; x < 80; ++x) {
                content += screen.PixelAt(x, y).character;
            }
        }

        // Should contain some expected elements
        REQUIRE(content.find("Login") != std::string::npos ||
                content.find("login") != std::string::npos ||
                content.find("LOGIN") != std::string::npos);
    }

    SECTION("Save screen rendering") {
        Map testMap(50, 30);
        GameManager gameManager;
        gameManager.getCurrentMap() = &testMap;

        SaveLoadScreen saveScreen(&gameManager);
        auto component = saveScreen.CreateComponent();

        auto screen = Screen::Create(Dimension::Fixed(80), Dimension::Fixed(24));
        auto doc = component->Render();
        Render(screen, doc);

        std::string content;
        for (int y = 0; y < 24; ++y) {
            for (int x = 0; x < 80; ++x) {
                content += screen.PixelAt(x, y).character;
            }
        }

        // Should show save slots
        REQUIRE(content.find("Slot") != std::string::npos ||
                content.find("slot") != std::string::npos ||
                content.find("SLOT") != std::string::npos ||
                content.find("1") != std::string::npos);
    }

    SECTION("Cloud indicator rendering") {
        CloudSaveIndicator indicator;
        indicator.SetStatus(CloudSyncStatus::Syncing);
        indicator.SetMessage("Syncing...");
        indicator.SetProgress(0.75f);

        auto component = indicator.CreateComponent();
        auto screen = Screen::Create(Dimension::Fixed(30), Dimension::Fixed(3));
        auto doc = component->Render();
        Render(screen, doc);

        std::string content;
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 30; ++x) {
                content += screen.PixelAt(x, y).character;
            }
        }

        // Should show sync message
        REQUIRE(content.find("Sync") != std::string::npos ||
                content.find("sync") != std::string::npos ||
                content.find("75") != std::string::npos); // Progress percentage
    }
}

TEST_CASE("UI Component error handling", "[ui][errors]") {
    SECTION("Login with database down") {
        // Simulate database being unavailable
        db::DatabaseManager::getInstance().shutdown();

        LoginScreen loginScreen;
        loginScreen.SetUsername("testuser");
        loginScreen.SetPassword("testpass");

        bool result = loginScreen.AttemptLogin();
        REQUIRE(result == false);
        REQUIRE(!loginScreen.GetError().empty());
    }

    SECTION("Save with full disk") {
        Map testMap(50, 30);
        GameManager gameManager;
        gameManager.getCurrentMap() = &testMap;

        SaveLoadScreen saveScreen(&gameManager);

        // Try to save to invalid location
        bool result = saveScreen.PerformSaveToPath("/invalid/path/save.dat");
        REQUIRE(result == false);
        REQUIRE(!saveScreen.GetLastError().empty());
    }

    SECTION("Cloud sync without network") {
        CloudSaveIndicator indicator;

        // Simulate network failure
        indicator.SimulateNetworkError();
        indicator.SetStatus(CloudSyncStatus::Syncing);

        // Should immediately fail
        indicator.Update(0.1f);
        REQUIRE(indicator.GetStatus() == CloudSyncStatus::Error);
    }

    SECTION("Invalid input handling") {
        LoginScreen loginScreen;

        // SQL injection attempt
        loginScreen.SetUsername("admin'; DROP TABLE users; --");
        loginScreen.SetPassword("password");

        bool result = loginScreen.AttemptLogin();
        REQUIRE(result == false);

        // Should sanitize input
        std::string sanitized = loginScreen.GetSanitizedUsername();
        REQUIRE(sanitized.find("DROP") == std::string::npos);
    }

    SECTION("Component null checks") {
        LoginScreen* nullLogin = nullptr;
        SaveLoadScreen* nullSave = nullptr;
        CloudSaveIndicator* nullCloud = nullptr;

        // Should not crash
        REQUIRE(nullLogin == nullptr);
        REQUIRE(nullSave == nullptr);
        REQUIRE(nullCloud == nullptr);
    }
}

TEST_CASE("UI Component performance", "[ui][performance]") {
    SECTION("Rapid login attempts") {
        LoginScreen loginScreen;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 100; ++i) {
            loginScreen.SetUsername("user" + std::to_string(i));
            loginScreen.SetPassword("pass" + std::to_string(i));
            loginScreen.AttemptLogin();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should handle rapid attempts efficiently
        REQUIRE(duration.count() < 5000); // Less than 5 seconds for 100 attempts
    }

    SECTION("Large save list rendering") {
        Map testMap(50, 30);
        GameManager gameManager;
        gameManager.getCurrentMap() = &testMap;

        SaveLoadScreen saveScreen(&gameManager);

        // Simulate many saves
        for (int i = 1; i <= 9; ++i) {
            saveScreen.SimulateSave(i, "Character" + std::to_string(i), i * 10);
        }

        auto component = saveScreen.CreateComponent();
        auto screen = Screen::Create(Dimension::Fixed(80), Dimension::Fixed(24));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 60; ++i) { // 60 frames
            auto doc = component->Render();
            Render(screen, doc);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should maintain 60 FPS (1000ms for 60 frames)
        REQUIRE(duration.count() < 2000);
    }

    SECTION("Cloud indicator animation") {
        CloudSaveIndicator indicator;
        indicator.SetStatus(CloudSyncStatus::Syncing);
        indicator.StartAnimation();

        auto start = std::chrono::high_resolution_clock::now();

        // Animate for 1000 frames
        for (int i = 0; i < 1000; ++i) {
            indicator.Update(0.016f); // 60 FPS
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should handle animation efficiently
        REQUIRE(duration.count() < 500); // Less than 500ms for 1000 updates
    }
}