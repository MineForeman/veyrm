#include <catch2/catch_test_macros.hpp>
#include "login_screen.h"
#include "auth/authentication_service.h"
#include "db/database_manager.h"
#include <memory>

// Mock authentication service for testing
class MockAuthenticationService : public auth::AuthenticationService {
private:
    bool should_succeed = true;
    bool should_require_verification = false;

public:
    MockAuthenticationService(std::shared_ptr<db::DatabaseManager> db)
        : AuthenticationService(db) {}

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

TEST_CASE("LoginScreen Tests", "[login_screen]") {
    // Create mock database and auth service
    auto mock_db = std::make_shared<db::DatabaseManager>("mock://connection");
    auto mock_auth = std::make_unique<MockAuthenticationService>(mock_db);
    auto* auth_ptr = mock_auth.get();

    SECTION("Constructor and initial state") {
        LoginScreen login_screen(*mock_auth);

        // Test that the object is constructed properly
        // Since we can't easily test the internal state without running the UI loop,
        // we focus on testing the methods that don't require UI interaction
    }

    SECTION("Email validation") {
        LoginScreen login_screen(*mock_auth);

        // Test valid emails
        REQUIRE(login_screen.isValidEmail("test@example.com"));
        REQUIRE(login_screen.isValidEmail("user.name+tag@domain.co.uk"));
        REQUIRE(login_screen.isValidEmail("a@b.co"));

        // Test invalid emails
        REQUIRE_FALSE(login_screen.isValidEmail(""));
        REQUIRE_FALSE(login_screen.isValidEmail("invalid"));
        REQUIRE_FALSE(login_screen.isValidEmail("@domain.com"));
        REQUIRE_FALSE(login_screen.isValidEmail("user@"));
        REQUIRE_FALSE(login_screen.isValidEmail("user@domain"));
        REQUIRE_FALSE(login_screen.isValidEmail("user.domain.com"));
    }

    SECTION("Form clearing") {
        LoginScreen login_screen(*mock_auth);

        // Set some test data
        login_screen.username_input = "test_user";
        login_screen.password_input = "test_pass";
        login_screen.remember_me = true;
        login_screen.reg_username_input = "reg_user";
        login_screen.reg_email_input = "reg@test.com";
        login_screen.error_message = "Test error";
        login_screen.show_error = true;

        // Clear forms
        login_screen.clearForms();

        // Verify everything is cleared
        REQUIRE(login_screen.username_input.empty());
        REQUIRE(login_screen.password_input.empty());
        REQUIRE_FALSE(login_screen.remember_me);
        REQUIRE(login_screen.reg_username_input.empty());
        REQUIRE(login_screen.reg_email_input.empty());
        REQUIRE(login_screen.error_message.empty());
        REQUIRE_FALSE(login_screen.show_error);
        REQUIRE_FALSE(login_screen.show_success);
    }

    SECTION("Mode switching") {
        LoginScreen login_screen(*mock_auth);

        // Test switching modes
        login_screen.switchMode(LoginScreen::Mode::REGISTER);
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::REGISTER);

        login_screen.switchMode(LoginScreen::Mode::FORGOT_PASSWORD);
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::FORGOT_PASSWORD);

        login_screen.switchMode(LoginScreen::Mode::VERIFY_EMAIL);
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::VERIFY_EMAIL);
    }

    SECTION("Error and success message handling") {
        LoginScreen login_screen(*mock_auth);

        // Test showing error
        login_screen.showError("Test error message");
        REQUIRE(login_screen.error_message == "Test error message");
        REQUIRE(login_screen.show_error);
        REQUIRE_FALSE(login_screen.show_success);

        // Test showing success (should clear error)
        login_screen.showSuccess("Test success message");
        REQUIRE(login_screen.status_message == "Test success message");
        REQUIRE(login_screen.show_success);
        REQUIRE_FALSE(login_screen.show_error);
    }

    SECTION("Login handling - successful login") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(true);

        // Set up login form data
        login_screen.username_input = "test_user";
        login_screen.password_input = "test_pass";
        login_screen.remember_me = true;

        // Test callback
        bool callback_called = false;
        uint32_t callback_user_id = 0;
        std::string callback_session_token;

        login_screen.on_login_success = [&](uint32_t user_id, const std::string& session_token) {
            callback_called = true;
            callback_user_id = user_id;
            callback_session_token = session_token;
        };

        // Handle login
        login_screen.handleLogin();

        // Verify success
        REQUIRE(login_screen.user_id == 123);
        REQUIRE(login_screen.session_token == "mock_session_token");
        REQUIRE(login_screen.refresh_token == "mock_refresh_token");
        REQUIRE(callback_called);
        REQUIRE(callback_user_id == 123);
        REQUIRE(callback_session_token == "mock_session_token");
    }

    SECTION("Login handling - failed login") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(false);

        // Set up login form data
        login_screen.username_input = "test_user";
        login_screen.password_input = "wrong_pass";

        // Handle login
        login_screen.handleLogin();

        // Verify failure
        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Mock login failure");
    }

    SECTION("Login handling - empty credentials") {
        LoginScreen login_screen(*mock_auth);

        // Test with empty username
        login_screen.username_input = "";
        login_screen.password_input = "test_pass";
        login_screen.handleLogin();

        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Please enter username and password");

        // Test with empty password
        login_screen.username_input = "test_user";
        login_screen.password_input = "";
        login_screen.handleLogin();

        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Please enter username and password");
    }

    SECTION("Registration handling - successful registration") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(true, false);

        // Set up registration form data
        login_screen.reg_username_input = "new_user";
        login_screen.reg_email_input = "new@test.com";
        login_screen.reg_password_input = "new_pass";
        login_screen.reg_confirm_password_input = "new_pass";

        // Handle registration
        login_screen.handleRegistration();

        // Verify success
        REQUIRE(login_screen.user_id == 456);
        REQUIRE(login_screen.show_success);
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::LOGIN);
    }

    SECTION("Registration handling - with email verification") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(true, true);

        // Set up registration form data
        login_screen.reg_username_input = "new_user";
        login_screen.reg_email_input = "new@test.com";
        login_screen.reg_password_input = "new_pass";
        login_screen.reg_confirm_password_input = "new_pass";

        // Handle registration
        login_screen.handleRegistration();

        // Verify success with verification
        REQUIRE(login_screen.user_id == 456);
        REQUIRE(login_screen.show_success);
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::VERIFY_EMAIL);
        REQUIRE(login_screen.verify_token_input == "mock_verification_token");
    }

    SECTION("Registration handling - password mismatch") {
        LoginScreen login_screen(*mock_auth);

        // Set up registration form data with mismatched passwords
        login_screen.reg_username_input = "new_user";
        login_screen.reg_email_input = "new@test.com";
        login_screen.reg_password_input = "new_pass";
        login_screen.reg_confirm_password_input = "different_pass";

        // Handle registration
        login_screen.handleRegistration();

        // Verify error
        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Passwords do not match");
    }

    SECTION("Registration handling - invalid email") {
        LoginScreen login_screen(*mock_auth);

        // Set up registration form data with invalid email
        login_screen.reg_username_input = "new_user";
        login_screen.reg_email_input = "invalid_email";
        login_screen.reg_password_input = "new_pass";
        login_screen.reg_confirm_password_input = "new_pass";

        // Handle registration
        login_screen.handleRegistration();

        // Verify error
        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Invalid email format");
    }

    SECTION("Registration handling - empty fields") {
        LoginScreen login_screen(*mock_auth);

        // Test with empty username
        login_screen.reg_username_input = "";
        login_screen.reg_email_input = "test@test.com";
        login_screen.reg_password_input = "pass";
        login_screen.reg_confirm_password_input = "pass";

        login_screen.handleRegistration();

        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Please fill in all fields");
    }

    SECTION("Email verification handling") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(true);

        // Test successful verification
        login_screen.verify_token_input = "mock_verification_token";
        login_screen.handleEmailVerification();

        REQUIRE(login_screen.show_success);
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::LOGIN);

        // Test failed verification
        auth_ptr->setMockBehavior(false);
        login_screen.verify_token_input = "invalid_token";
        login_screen.handleEmailVerification();

        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Invalid or expired verification token");

        // Test empty token
        login_screen.verify_token_input = "";
        login_screen.handleEmailVerification();

        REQUIRE(login_screen.show_error);
        REQUIRE(login_screen.error_message == "Please enter verification token");
    }

    SECTION("Result setting") {
        LoginScreen login_screen(*mock_auth);

        // Test setting different results
        login_screen.exitScreen(LoginScreen::Result::SUCCESS);
        REQUIRE(login_screen.result == LoginScreen::Result::SUCCESS);

        login_screen.exitScreen(LoginScreen::Result::CANCELLED);
        REQUIRE(login_screen.result == LoginScreen::Result::CANCELLED);
    }
}

TEST_CASE("LoginScreen UI Component Creation", "[login_screen][ui]") {
    auto mock_db = std::make_shared<db::DatabaseManager>("mock://connection");
    auto mock_auth = std::make_unique<MockAuthenticationService>(mock_db);

    SECTION("Component creation doesn't crash") {
        LoginScreen login_screen(*mock_auth);

        // Test that component creation methods don't crash
        // We can't easily test the UI components without a full FTXUI test setup,
        // but we can at least verify the methods can be called

        auto login_form = login_screen.createLoginForm();
        REQUIRE(login_form != nullptr);

        auto reg_form = login_screen.createRegistrationForm();
        REQUIRE(reg_form != nullptr);

        auto reset_form = login_screen.createPasswordResetForm();
        REQUIRE(reset_form != nullptr);

        auto verify_form = login_screen.createEmailVerificationForm();
        REQUIRE(verify_form != nullptr);
    }
}

TEST_CASE("LoginScreen Integration with AuthenticationService", "[login_screen][integration]") {
    auto mock_db = std::make_shared<db::DatabaseManager>("mock://connection");
    auto mock_auth = std::make_unique<MockAuthenticationService>(mock_db);
    auto* auth_ptr = mock_auth.get();

    SECTION("Full login flow") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(true);

        // Simulate user input
        login_screen.username_input = "testuser";
        login_screen.password_input = "testpass";
        login_screen.remember_me = true;

        // Track login callback
        bool login_success = false;
        login_screen.on_login_success = [&](uint32_t user_id, const std::string& token) {
            login_success = true;
        };

        // Perform login
        login_screen.handleLogin();

        // Verify results
        REQUIRE(login_success);
        REQUIRE(login_screen.user_id == 123);
        REQUIRE_FALSE(login_screen.session_token.empty());
    }

    SECTION("Full registration flow with verification") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(true, true);

        // Registration
        login_screen.reg_username_input = "newuser";
        login_screen.reg_email_input = "new@example.com";
        login_screen.reg_password_input = "password123";
        login_screen.reg_confirm_password_input = "password123";

        login_screen.handleRegistration();

        // Should switch to verification mode
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::VERIFY_EMAIL);
        REQUIRE_FALSE(login_screen.verify_token_input.empty());

        // Verify email
        login_screen.handleEmailVerification();

        // Should switch back to login mode
        REQUIRE(login_screen.current_mode == LoginScreen::Mode::LOGIN);
        REQUIRE(login_screen.show_success);
    }

    SECTION("Password reset flow") {
        LoginScreen login_screen(*mock_auth);
        auth_ptr->setMockBehavior(true);

        // Switch to password reset mode
        login_screen.switchMode(LoginScreen::Mode::FORGOT_PASSWORD);

        // Request reset token (this is handled inline in the password reset form)
        login_screen.reset_email_input = "user@example.com";

        // Simulate the request button action from createPasswordResetForm
        auto token = auth_ptr->requestPasswordReset(login_screen.reset_email_input);
        REQUIRE(token.has_value());
        login_screen.reset_token_input = token.value();

        // Reset password
        login_screen.reset_new_password_input = "newpassword123";
        bool reset_success = auth_ptr->resetPassword(login_screen.reset_token_input, login_screen.reset_new_password_input);
        REQUIRE(reset_success);
    }
}