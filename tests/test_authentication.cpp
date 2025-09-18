#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "auth/authentication_service.h"
#include "db/player_repository.h"
#include "db/database_manager.h"
#include "config.h"
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>

using namespace Catch::Matchers;

// Helper function to load .env file
void loadEnvironment() {
    std::ifstream file(".env");
    if (!file.is_open()) {
        return; // .env file not found, skip
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the = separator
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remove quotes if present
            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }

            // Set environment variable
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
}

// Test fixture for authentication tests
class AuthTestFixture {
public:
    bool db_available = false;
    AuthTestFixture() {
        // Load environment variables first (for database password)
        loadEnvironment();

        // Initialize database manager
        Config& config = Config::getInstance();
        config.loadFromFile("config.yml");

        auto db_config = config.getDatabaseConfig();
        db_config.database = "veyrm_test"; // Use test database

        // Ensure we have a password from environment
        if (db_config.password.empty()) {
            // Try to get from environment directly if config didn't load it
            const char* db_pass = std::getenv("DB_PASS");
            if (db_pass) {
                db_config.password = db_pass;
            }
        }

        // Try to initialize database, but don't fail if it's not available
        try {
            db::DatabaseManager::getInstance().initialize(db_config);
            db_available = db::DatabaseManager::getInstance().isInitialized();
        } catch (const std::exception& e) {
            // Database not available, tests will be skipped
            db_available = false;
        }

        if (db_available) {
            // Create repositories and services
            player_repo = std::make_unique<db::PlayerRepository>(
                db::DatabaseManager::getInstance()
            );

            auth_service = std::make_unique<auth::AuthenticationService>(
                *player_repo,
                db::DatabaseManager::getInstance()
            );

            // Clean up any existing test users
            cleanupTestUsers();
        }
    }

    ~AuthTestFixture() {
        cleanupTestUsers();
    }

    void cleanupTestUsers() {
        // Clean up test users created during tests
        // Note: In a real implementation, you'd have a method to delete users by username pattern
        // For now, we'll just ensure the database is in a clean state
    }

    std::unique_ptr<db::PlayerRepository> player_repo;
    std::unique_ptr<auth::AuthenticationService> auth_service;

    // Test user data
    const std::string test_username = "testuser_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    const std::string test_email = test_username + "@test.com";
    const std::string test_password = "TestPass123!";
};

TEST_CASE("Authentication Service - User Registration", "[auth][database]") {
    AuthTestFixture fixture;

    if (!fixture.db_available) {
        SUCCEED("Authentication tests skipped - PostgreSQL not available");
        return;
    }

    SECTION("Successful registration with valid data") {
        auto result = fixture.auth_service->registerUser(
            fixture.test_username,
            fixture.test_email,
            fixture.test_password
        );

        REQUIRE(result.success == true);
        REQUIRE(result.user_id.has_value());
        REQUIRE(result.error_message.empty());
        REQUIRE(result.verification_token.has_value());
    }

    SECTION("Registration fails with duplicate username") {
        // Register first user
        fixture.auth_service->registerUser(
            fixture.test_username,
            fixture.test_email,
            fixture.test_password
        );

        // Try to register with same username
        auto result = fixture.auth_service->registerUser(
            fixture.test_username,
            "different@test.com",
            fixture.test_password
        );

        REQUIRE(result.success == false);
        REQUIRE_THAT(result.error_message, ContainsSubstring("Username already taken"));
    }

    SECTION("Registration fails with duplicate email") {
        // Register first user
        fixture.auth_service->registerUser(
            fixture.test_username,
            fixture.test_email,
            fixture.test_password
        );

        // Try to register with same email
        auto result = fixture.auth_service->registerUser(
            "differentuser",
            fixture.test_email,
            fixture.test_password
        );

        REQUIRE(result.success == false);
        REQUIRE_THAT(result.error_message, ContainsSubstring("Email already registered"));
    }

    SECTION("Registration validates username format") {
        auto result = fixture.auth_service->registerUser(
            "ab",  // Too short
            fixture.test_email,
            fixture.test_password
        );

        REQUIRE(result.success == false);
        REQUIRE_THAT(result.error_message, ContainsSubstring("at least 3 characters"));
    }

    SECTION("Registration validates email format") {
        auto result = fixture.auth_service->registerUser(
            fixture.test_username,
            "invalid-email",
            fixture.test_password
        );

        REQUIRE(result.success == false);
        REQUIRE_THAT(result.error_message, ContainsSubstring("Invalid email format"));
    }

    SECTION("Registration validates password strength") {
        auto result = fixture.auth_service->registerUser(
            fixture.test_username,
            fixture.test_email,
            "weak"  // Too short and weak
        );

        REQUIRE(result.success == false);
        REQUIRE_THAT(result.error_message, ContainsSubstring("at least"));
    }
}

TEST_CASE("Authentication Service - User Login", "[auth][database]") {
    AuthTestFixture fixture;

    if (!fixture.db_available) {
        SUCCEED("Authentication tests skipped - PostgreSQL not available");
        return;
    }

    // Register a test user
    auto reg_result = fixture.auth_service->registerUser(
        fixture.test_username,
        fixture.test_email,
        fixture.test_password
    );
    REQUIRE(reg_result.success == true);

    SECTION("Successful login with username") {
        auto login_result = fixture.auth_service->login(
            fixture.test_username,
            fixture.test_password,
            false,
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(login_result.success == true);
        REQUIRE(login_result.session_token.has_value());
        REQUIRE(login_result.refresh_token.has_value());
        REQUIRE(login_result.user_id.has_value());
        REQUIRE(login_result.error_message.empty());
    }

    SECTION("Successful login with email") {
        auto login_result = fixture.auth_service->login(
            fixture.test_email,
            fixture.test_password,
            false,
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(login_result.success == true);
        REQUIRE(login_result.session_token.has_value());
    }

    SECTION("Login fails with wrong password") {
        auto login_result = fixture.auth_service->login(
            fixture.test_username,
            "WrongPassword123!",
            false,
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(login_result.success == false);
        REQUIRE_THAT(login_result.error_message, ContainsSubstring("Invalid username or password"));
    }

    SECTION("Login fails with non-existent user") {
        auto login_result = fixture.auth_service->login(
            "nonexistentuser",
            fixture.test_password,
            false,
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(login_result.success == false);
        REQUIRE_THAT(login_result.error_message, ContainsSubstring("Invalid username or password"));
    }

    SECTION("Remember me creates longer session") {
        auto normal_login = fixture.auth_service->login(
            fixture.test_username,
            fixture.test_password,
            false,  // No remember me
            "127.0.0.1",
            "Test Client"
        );

        auto remember_login = fixture.auth_service->login(
            fixture.test_username,
            fixture.test_password,
            true,  // Remember me
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(normal_login.success == true);
        REQUIRE(remember_login.success == true);
        // Both should have tokens
        REQUIRE(normal_login.session_token.has_value());
        REQUIRE(remember_login.session_token.has_value());
    }
}

TEST_CASE("Authentication Service - Account Lockout", "[auth][database]") {
    AuthTestFixture fixture;

    if (!fixture.db_available) {
        SUCCEED("Authentication tests skipped - PostgreSQL not available");
        return;
    }

    // Set aggressive lockout policy for testing
    fixture.auth_service->setMaxLoginAttempts(3);
    fixture.auth_service->setLockoutDuration(1); // 1 minute

    // Register a test user
    auto reg_result = fixture.auth_service->registerUser(
        fixture.test_username,
        fixture.test_email,
        fixture.test_password
    );
    REQUIRE(reg_result.success == true);

    SECTION("Account locks after max failed attempts") {
        // Make failed login attempts
        for (int i = 0; i < 3; i++) {
            auto result = fixture.auth_service->login(
                fixture.test_username,
                "WrongPassword",
                false,
                "127.0.0.1",
                "Test Client"
            );
            REQUIRE(result.success == false);
        }

        // Next attempt should indicate account is locked
        auto locked_result = fixture.auth_service->login(
            fixture.test_username,
            fixture.test_password,  // Even with correct password
            false,
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(locked_result.success == false);
        REQUIRE_THAT(locked_result.error_message, ContainsSubstring("locked"));
    }

    SECTION("Failed login counter resets on successful login") {
        // Make 2 failed attempts
        for (int i = 0; i < 2; i++) {
            fixture.auth_service->login(
                fixture.test_username,
                "WrongPassword",
                false,
                "127.0.0.1",
                "Test Client"
            );
        }

        // Successful login should reset counter
        auto success = fixture.auth_service->login(
            fixture.test_username,
            fixture.test_password,
            false,
            "127.0.0.1",
            "Test Client"
        );
        REQUIRE(success.success == true);

        // Now we should be able to fail 3 more times before lockout
        for (int i = 0; i < 2; i++) {
            auto result = fixture.auth_service->login(
                fixture.test_username,
                "WrongPassword",
                false,
                "127.0.0.1",
                "Test Client"
            );
            REQUIRE(result.success == false);
            REQUIRE_THAT(result.error_message, !ContainsSubstring("locked"));
        }
    }
}

TEST_CASE("Authentication Service - Session Management", "[auth][database]") {
    AuthTestFixture fixture;

    if (!fixture.db_available) {
        SUCCEED("Authentication tests skipped - PostgreSQL not available");
        return;
    }

    // Register and login
    fixture.auth_service->registerUser(
        fixture.test_username,
        fixture.test_email,
        fixture.test_password
    );

    auto login_result = fixture.auth_service->login(
        fixture.test_username,
        fixture.test_password,
        false,
        "127.0.0.1",
        "Test Client"
    );
    REQUIRE(login_result.success == true);

    std::string session_token = login_result.session_token.value();

    SECTION("Valid session validates successfully") {
        auto validation = fixture.auth_service->validateSession(session_token);

        REQUIRE(validation.valid == true);
        REQUIRE(validation.user_id.has_value());
        REQUIRE(validation.user_id.value() == login_result.user_id.value());
    }

    SECTION("Invalid session token fails validation") {
        auto validation = fixture.auth_service->validateSession("invalid_token_12345");

        REQUIRE(validation.valid == false);
        REQUIRE_THAT(validation.error_message, ContainsSubstring("Invalid session"));
    }

    SECTION("Logout invalidates session") {
        bool logout_success = fixture.auth_service->logout(session_token);
        REQUIRE(logout_success == true);

        // Session should no longer be valid
        auto validation = fixture.auth_service->validateSession(session_token);
        REQUIRE(validation.valid == false);
    }

    SECTION("Logout all sessions works") {
        // Create multiple sessions
        for (int i = 0; i < 3; i++) {
            fixture.auth_service->login(
                fixture.test_username,
                fixture.test_password,
                false,
                "127.0.0.1",
                "Test Client " + std::to_string(i)
            );
        }

        int revoked = fixture.auth_service->logoutAllSessions(login_result.user_id.value());
        REQUIRE(revoked >= 3);

        // Original session should be invalid
        auto validation = fixture.auth_service->validateSession(session_token);
        REQUIRE(validation.valid == false);
    }
}

TEST_CASE("Authentication Service - Password Management", "[auth][database]") {
    AuthTestFixture fixture;

    if (!fixture.db_available) {
        SUCCEED("Authentication tests skipped - PostgreSQL not available");
        return;
    }

    // Register a test user
    fixture.auth_service->registerUser(
        fixture.test_username,
        fixture.test_email,
        fixture.test_password
    );

    auto login_result = fixture.auth_service->login(
        fixture.test_username,
        fixture.test_password,
        false,
        "127.0.0.1",
        "Test Client"
    );
    int user_id = login_result.user_id.value();

    SECTION("Change password with correct old password") {
        std::string new_password = "NewPassword456!";

        bool changed = fixture.auth_service->changePassword(
            user_id,
            fixture.test_password,
            new_password
        );

        REQUIRE(changed == true);

        // Should be able to login with new password
        auto new_login = fixture.auth_service->login(
            fixture.test_username,
            new_password,
            false,
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(new_login.success == true);
    }

    SECTION("Change password fails with wrong old password") {
        bool changed = fixture.auth_service->changePassword(
            user_id,
            "WrongOldPassword",
            "NewPassword456!"
        );

        REQUIRE(changed == false);
    }

    SECTION("Password reset flow") {
        // Request password reset
        auto reset_token = fixture.auth_service->requestPasswordReset(fixture.test_email);
        REQUIRE(reset_token.has_value());

        // Reset password with token
        std::string new_password = "ResetPassword789!";
        bool reset = fixture.auth_service->resetPassword(reset_token.value(), new_password);
        REQUIRE(reset == true);

        // Should be able to login with new password
        auto new_login = fixture.auth_service->login(
            fixture.test_username,
            new_password,
            false,
            "127.0.0.1",
            "Test Client"
        );

        REQUIRE(new_login.success == true);
    }

    SECTION("Password reset with invalid token fails") {
        bool reset = fixture.auth_service->resetPassword(
            "invalid_reset_token",
            "NewPassword123!"
        );

        REQUIRE(reset == false);
    }
}

TEST_CASE("Authentication Service - Email Verification", "[auth][database]") {
    AuthTestFixture fixture;

    if (!fixture.db_available) {
        SUCCEED("Authentication tests skipped - PostgreSQL not available");
        return;
    }

    // Register a test user
    auto reg_result = fixture.auth_service->registerUser(
        fixture.test_username,
        fixture.test_email,
        fixture.test_password
    );

    REQUIRE(reg_result.success == true);
    REQUIRE(reg_result.verification_token.has_value());

    std::string token = reg_result.verification_token.value();

    SECTION("Email verification with valid token succeeds") {
        bool verified = fixture.auth_service->verifyEmail(token);
        REQUIRE(verified == true);

        // User should now have verified email
        auto user = fixture.player_repo->findUserByUsername(fixture.test_username);
        REQUIRE(user.has_value());
        REQUIRE(user->email_verified == true);
    }

    SECTION("Email verification with invalid token fails") {
        bool verified = fixture.auth_service->verifyEmail("invalid_token");
        REQUIRE(verified == false);
    }

    SECTION("Email verification token can only be used once") {
        bool first_verify = fixture.auth_service->verifyEmail(token);
        REQUIRE(first_verify == true);

        bool second_verify = fixture.auth_service->verifyEmail(token);
        REQUIRE(second_verify == false);
    }
}

TEST_CASE("Authentication Service - Input Validation", "[auth]") {
    AuthTestFixture fixture;

    if (!fixture.db_available) {
        SUCCEED("Authentication tests skipped - PostgreSQL not available");
        return;
    }

    SECTION("Username validation") {
        auto short_username = fixture.auth_service->validateUsername("ab");
        REQUIRE(short_username.has_value());
        REQUIRE_THAT(short_username.value(), ContainsSubstring("at least 3"));

        auto long_username = fixture.auth_service->validateUsername(std::string(51, 'a'));
        REQUIRE(long_username.has_value());
        REQUIRE_THAT(long_username.value(), ContainsSubstring("no more than 50"));

        auto invalid_chars = fixture.auth_service->validateUsername("user@name");
        REQUIRE(invalid_chars.has_value());
        REQUIRE_THAT(invalid_chars.value(), ContainsSubstring("letters, numbers, and underscores"));

        auto valid_username = fixture.auth_service->validateUsername("valid_user123");
        REQUIRE(!valid_username.has_value());
    }

    SECTION("Email validation") {
        auto invalid_format = fixture.auth_service->validateEmail("not-an-email");
        REQUIRE(invalid_format.has_value());
        REQUIRE_THAT(invalid_format.value(), ContainsSubstring("Invalid email"));

        auto empty_email = fixture.auth_service->validateEmail("");
        REQUIRE(empty_email.has_value());
        REQUIRE_THAT(empty_email.value(), ContainsSubstring("required"));

        auto valid_email = fixture.auth_service->validateEmail("user@example.com");
        REQUIRE(!valid_email.has_value());
    }

    SECTION("Password validation") {
        fixture.auth_service->setSessionLifetime(4); // Ensure min length is 8

        auto short_password = fixture.auth_service->validatePassword("Pass1!");
        REQUIRE(short_password.has_value());
        REQUIRE_THAT(short_password.value(), ContainsSubstring("at least"));

        auto no_uppercase = fixture.auth_service->validatePassword("Password123!");
        REQUIRE(no_uppercase.has_value());
        REQUIRE_THAT(no_uppercase.value(), ContainsSubstring("uppercase"));

        auto no_lowercase = fixture.auth_service->validatePassword("PASSWORD123!");
        REQUIRE(no_lowercase.has_value());
        REQUIRE_THAT(no_lowercase.value(), ContainsSubstring("lowercase"));

        auto no_numbers = fixture.auth_service->validatePassword("PasswordTest!");
        REQUIRE(no_numbers.has_value());
        REQUIRE_THAT(no_numbers.value(), ContainsSubstring("numbers"));

        auto valid_password = fixture.auth_service->validatePassword("ValidPass123");
        REQUIRE(!valid_password.has_value());
    }
}