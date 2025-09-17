#include <catch2/catch_test_macros.hpp>
#include "auth/validation_service.h"
#include "auth/login_controller.h"
#include "auth/authentication_service.h"
#include "db/database_manager.h"
#include <memory>

// Mock AuthenticationService for testing
class MockAuthenticationService : public auth::AuthenticationService {
private:
    bool should_succeed = true;
    bool should_require_verification = false;

public:
    MockAuthenticationService() : AuthenticationService(nullptr) {}

    void setMockBehavior(bool succeed, bool require_verification = false) {
        should_succeed = succeed;
        should_require_verification = require_verification;
    }

    auth::LoginResult login(const std::string& username_or_email,
                           const std::string& password,
                           bool remember_me,
                           const std::string& ip_address,
                           const std::string& user_agent) override {
        auth::LoginResult result;
        result.success = should_succeed;

        if (should_succeed) {
            result.user_id = 123;
            result.session_token = "mock_session_token";
            result.refresh_token = "mock_refresh_token";
        } else {
            result.error_message = "Mock login failure";
        }

        return result;
    }

    auth::RegistrationResult registerUser(const std::string& username,
                                        const std::string& email,
                                        const std::string& password) override {
        auth::RegistrationResult result;
        result.success = should_succeed;

        if (should_succeed) {
            result.user_id = 456;
            if (should_require_verification) {
                result.verification_token = "mock_verification_token";
            }
        } else {
            result.error_message = "Mock registration failure";
        }

        return result;
    }

    bool verifyEmail(const std::string& token) override {
        return should_succeed && token == "mock_verification_token";
    }

    std::optional<std::string> requestPasswordReset(const std::string& email) override {
        if (should_succeed) {
            return "mock_reset_token";
        }
        return std::nullopt;
    }

    bool resetPassword(const std::string& token, const std::string& new_password) override {
        return should_succeed && token == "mock_reset_token";
    }
};

TEST_CASE("ValidationService Tests", "[validation][business_logic]") {
    auth::ValidationService validator;

    SECTION("Email validation") {
        // Valid emails
        REQUIRE_FALSE(validator.validateEmail("test@example.com").has_value());
        REQUIRE_FALSE(validator.validateEmail("user.name+tag@domain.co.uk").has_value());
        REQUIRE_FALSE(validator.validateEmail("a@b.co").has_value());

        // Invalid emails
        REQUIRE(validator.validateEmail("").has_value());
        REQUIRE(validator.validateEmail("invalid").has_value());
        REQUIRE(validator.validateEmail("@domain.com").has_value());
        REQUIRE(validator.validateEmail("user@").has_value());
        REQUIRE(validator.validateEmail("user@domain").has_value());
        REQUIRE(validator.validateEmail("user.domain.com").has_value());
    }

    SECTION("Password validation") {
        // Valid passwords
        REQUIRE_FALSE(validator.validatePassword("password123").has_value());
        REQUIRE_FALSE(validator.validatePassword("123456").has_value());

        // Invalid passwords
        REQUIRE(validator.validatePassword("").has_value());
        REQUIRE(validator.validatePassword("12345").has_value()); // Too short
    }

    SECTION("Username validation") {
        // Valid usernames
        REQUIRE_FALSE(validator.validateUsername("user123").has_value());
        REQUIRE_FALSE(validator.validateUsername("test_user").has_value());
        REQUIRE_FALSE(validator.validateUsername("ABC").has_value());

        // Invalid usernames
        REQUIRE(validator.validateUsername("").has_value()); // Empty
        REQUIRE(validator.validateUsername("ab").has_value()); // Too short
        REQUIRE(validator.validateUsername("a").has_value()); // Too short
        REQUIRE(validator.validateUsername("this_is_a_very_long_username").has_value()); // Too long
        REQUIRE(validator.validateUsername("user@name").has_value()); // Invalid characters
        REQUIRE(validator.validateUsername("user name").has_value()); // Spaces
        REQUIRE(validator.validateUsername("user-name").has_value()); // Dashes
    }

    SECTION("Password confirmation validation") {
        REQUIRE_FALSE(validator.validatePasswordConfirmation("password", "password").has_value());
        REQUIRE(validator.validatePasswordConfirmation("password", "different").has_value());
    }

    SECTION("Login credentials validation") {
        REQUIRE_FALSE(validator.validateLoginCredentials("user", "pass").has_value());
        REQUIRE(validator.validateLoginCredentials("", "pass").has_value());
        REQUIRE(validator.validateLoginCredentials("user", "").has_value());
        REQUIRE(validator.validateLoginCredentials("", "").has_value());
    }

    SECTION("Registration data validation") {
        // Valid registration
        REQUIRE_FALSE(validator.validateRegistrationData("user123", "test@example.com", "password123", "password123").has_value());

        // Invalid cases
        REQUIRE(validator.validateRegistrationData("", "test@example.com", "password123", "password123").has_value()); // Empty username
        REQUIRE(validator.validateRegistrationData("user123", "", "password123", "password123").has_value()); // Empty email
        REQUIRE(validator.validateRegistrationData("user123", "test@example.com", "", "password123").has_value()); // Empty password
        REQUIRE(validator.validateRegistrationData("user123", "test@example.com", "password123", "").has_value()); // Empty confirm
        REQUIRE(validator.validateRegistrationData("ab", "test@example.com", "password123", "password123").has_value()); // Invalid username
        REQUIRE(validator.validateRegistrationData("user123", "invalid-email", "password123", "password123").has_value()); // Invalid email
        REQUIRE(validator.validateRegistrationData("user123", "test@example.com", "12345", "12345").has_value()); // Invalid password
        REQUIRE(validator.validateRegistrationData("user123", "test@example.com", "password123", "different").has_value()); // Password mismatch
    }
}

TEST_CASE("LoginController Tests", "[login_controller][business_logic]") {
    auto mock_auth = std::make_unique<MockAuthenticationService>();
    auto* auth_ptr = mock_auth.get();

    auth::LoginController controller(*mock_auth);

    // Track view callbacks
    std::string last_error;
    std::string last_success;
    bool error_shown = false;
    bool success_shown = false;
    bool login_success_called = false;
    int login_user_id = 0;
    std::string login_session_token;

    auth::LoginController::ViewCallbacks callbacks;
    callbacks.showError = [&](const std::string& msg) {
        last_error = msg;
        error_shown = true;
    };
    callbacks.showSuccess = [&](const std::string& msg) {
        last_success = msg;
        success_shown = true;
    };
    callbacks.clearMessages = [&]() {
        error_shown = false;
        success_shown = false;
    };
    callbacks.onLoginSuccess = [&](int user_id, const std::string& token) {
        login_success_called = true;
        login_user_id = user_id;
        login_session_token = token;
    };

    controller.setViewCallbacks(callbacks);

    SECTION("Successful login") {
        auth_ptr->setMockBehavior(true);

        auth::LoginCredentials creds;
        creds.username = "testuser";
        creds.password = "testpass";
        creds.remember_me = true;

        controller.handleLogin(creds);

        REQUIRE(login_success_called);
        REQUIRE(login_user_id == 123);
        REQUIRE(login_session_token == "mock_session_token");
        REQUIRE_FALSE(error_shown);
    }

    SECTION("Failed login") {
        auth_ptr->setMockBehavior(false);

        auth::LoginCredentials creds;
        creds.username = "testuser";
        creds.password = "wrongpass";

        controller.handleLogin(creds);

        REQUIRE_FALSE(login_success_called);
        REQUIRE(error_shown);
        REQUIRE(last_error == "Mock login failure");
    }

    SECTION("Login with invalid credentials") {
        auth::LoginCredentials creds;
        creds.username = "";
        creds.password = "testpass";

        controller.handleLogin(creds);

        REQUIRE_FALSE(login_success_called);
        REQUIRE(error_shown);
        REQUIRE(last_error == "Please enter username and password");
    }

    SECTION("Successful registration") {
        auth_ptr->setMockBehavior(true, false);

        auth::RegistrationData data;
        data.username = "newuser";
        data.email = "new@test.com";
        data.password = "newpass123";
        data.confirm_password = "newpass123";

        controller.handleRegistration(data);

        REQUIRE(success_shown);
        REQUIRE(last_success == "Registration successful! You can now log in.");
    }

    SECTION("Registration with email verification") {
        auth_ptr->setMockBehavior(true, true);

        auth::RegistrationData data;
        data.username = "newuser";
        data.email = "new@test.com";
        data.password = "newpass123";
        data.confirm_password = "newpass123";

        controller.handleRegistration(data);

        REQUIRE(success_shown);
        REQUIRE(last_success == "Registration successful! Please check your email for verification.");
    }

    SECTION("Registration with password mismatch") {
        auth::RegistrationData data;
        data.username = "newuser";
        data.email = "new@test.com";
        data.password = "newpass123";
        data.confirm_password = "different";

        controller.handleRegistration(data);

        REQUIRE(error_shown);
        REQUIRE(last_error == "Passwords do not match");
    }

    SECTION("Registration with invalid email") {
        auth::RegistrationData data;
        data.username = "newuser";
        data.email = "invalid-email";
        data.password = "newpass123";
        data.confirm_password = "newpass123";

        controller.handleRegistration(data);

        REQUIRE(error_shown);
        REQUIRE(last_error == "Invalid email format");
    }

    SECTION("Password reset request") {
        auth_ptr->setMockBehavior(true);

        controller.handlePasswordResetRequest("user@example.com");

        REQUIRE(success_shown);
        REQUIRE(last_success == "Password reset instructions sent to your email.");
    }

    SECTION("Password reset request with invalid email") {
        controller.handlePasswordResetRequest("invalid-email");

        REQUIRE(error_shown);
        REQUIRE(last_error == "Invalid email format");
    }

    SECTION("Email verification") {
        auth_ptr->setMockBehavior(true);

        controller.handleEmailVerification("mock_verification_token");

        REQUIRE(success_shown);
        REQUIRE(last_success == "Email verified successfully! You can now log in.");
    }

    SECTION("Email verification with invalid token") {
        auth_ptr->setMockBehavior(false);

        controller.handleEmailVerification("invalid_token");

        REQUIRE(error_shown);
        REQUIRE(last_error == "Invalid or expired verification token");
    }
}

TEST_CASE("LoginController Edge Cases", "[login_controller][edge_cases]") {
    auto mock_auth = std::make_unique<MockAuthenticationService>();
    auth::LoginController controller(*mock_auth);

    SECTION("Operations without view callbacks set") {
        // Should not crash when callbacks aren't set
        auth::LoginCredentials creds;
        creds.username = "test";
        creds.password = "pass";

        controller.handleLogin(creds);
        // Should complete without crashing

        auth::RegistrationData data;
        data.username = "test";
        data.email = "test@test.com";
        data.password = "pass123";
        data.confirm_password = "pass123";

        controller.handleRegistration(data);
        // Should complete without crashing
    }

    SECTION("Empty string inputs") {
        std::string last_error;
        auth::LoginController::ViewCallbacks callbacks;
        callbacks.showError = [&](const std::string& msg) { last_error = msg; };
        controller.setViewCallbacks(callbacks);

        controller.handlePasswordResetRequest("");
        REQUIRE(last_error == "Email cannot be empty");

        controller.handleEmailVerification("");
        REQUIRE(last_error == "Please enter verification token");
    }
}