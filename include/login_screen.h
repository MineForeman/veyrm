#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <memory>
#include <functional>

namespace auth {
    class AuthenticationService;
    class ValidationService;
}

namespace db {
    class PlayerRepository;
    class DatabaseManager;
}

/**
 * @class LoginScreen
 * @brief FTXUI-based login and registration screen
 */
class LoginScreen {
public:
    enum class Mode {
        LOGIN,
        REGISTER,
        FORGOT_PASSWORD,
        VERIFY_EMAIL
    };

    enum class Result {
        SUCCESS,
        CANCELLED,
        FAILED
    };

    /**
     * @brief Constructor
     * @param auth_service Reference to authentication service
     */
    explicit LoginScreen(auth::AuthenticationService& auth_service);
    ~LoginScreen(); // Custom destructor needed for unique_ptr with forward declaration

    /**
     * @brief Run the login screen
     * @return Result of the login/registration attempt
     */
    Result run();

    /**
     * @brief Get the logged-in user ID
     * @return User ID if logged in successfully
     */
    int getUserId() const { return user_id; }

    /**
     * @brief Get the session token
     * @return Session token if logged in successfully
     */
    std::string getSessionToken() const { return session_token; }

    /**
     * @brief Set callback for successful login
     * @param callback Function to call on successful login
     */
    void setOnLoginSuccess(std::function<void(int, const std::string&)> callback) {
        on_login_success = callback;
    }

    /**
     * @brief Set the initial mode
     * @param mode Initial screen mode
     */
    void setMode(Mode mode) { current_mode = mode; }

private:
    auth::AuthenticationService& auth_service;
    std::unique_ptr<auth::ValidationService> validator;

    // UI State
    Mode current_mode = Mode::LOGIN;
    Result result = Result::CANCELLED;

    // Login form fields
    std::string username_input;
    std::string password_input;
    bool remember_me = false;

    // Registration form fields
    std::string reg_username_input;
    std::string reg_email_input;
    std::string reg_password_input;
    std::string reg_confirm_password_input;

    // Password reset fields
    std::string reset_email_input;
    std::string reset_token_input;
    std::string reset_new_password_input;

    // Email verification fields
    std::string verify_token_input;

    // Status and error messages
    std::string status_message;
    std::string error_message;
    bool show_error = false;
    bool show_success = false;

    // Result data
    int user_id = 0;
    std::string session_token;
    std::string refresh_token;

    // Callbacks
    std::function<void(int, const std::string&)> on_login_success;

    // FTXUI Components
    ftxui::Component container;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    // === Helper Functions ===

    /**
     * @brief Create the login form component
     * @return FTXUI component for login form
     */
    ftxui::Component createLoginForm();

    /**
     * @brief Create the registration form component
     * @return FTXUI component for registration form
     */
    ftxui::Component createRegistrationForm();

    /**
     * @brief Create the password reset form component
     * @return FTXUI component for password reset
     */
    ftxui::Component createPasswordResetForm();

    /**
     * @brief Create the email verification form component
     * @return FTXUI component for email verification
     */
    ftxui::Component createEmailVerificationForm();

    /**
     * @brief Handle login submission
     */
    void handleLogin();

    /**
     * @brief Handle registration submission
     */
    void handleRegistration();

    /**
     * @brief Handle password reset request
     */
    void handlePasswordReset();

    /**
     * @brief Handle email verification
     */
    void handleEmailVerification();

    /**
     * @brief Switch to a different mode
     * @param mode New mode to switch to
     */
    void switchMode(Mode mode);

    /**
     * @brief Show an error message
     * @param message Error message to display
     */
    void showError(const std::string& message);

    /**
     * @brief Show a success message
     * @param message Success message to display
     */
    void showSuccess(const std::string& message);

    /**
     * @brief Clear all form fields
     */
    void clearForms();


    /**
     * @brief Exit the screen
     * @param exit_result Result to set when exiting
     */
    void exitScreen(Result exit_result);
};