#pragma once
#include "auth/login_models.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <functional>

namespace ui {

/**
 * @brief Pure UI view for login (minimal business logic)
 * This class focuses only on UI rendering and user interaction
 */
class LoginView {
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
     * @brief Callbacks to business logic (dependency inversion)
     */
    struct ControllerCallbacks {
        std::function<void(const auth::LoginCredentials&)> onLogin;
        std::function<void(const auth::RegistrationData&)> onRegister;
        std::function<void(const std::string&)> onPasswordResetRequest;
        std::function<void(const std::string&, const std::string&)> onPasswordReset;
        std::function<void(const std::string&)> onEmailVerification;
        std::function<void()> onCancel;
    };

    /**
     * @brief Constructor
     */
    LoginView();

    /**
     * @brief Set controller callbacks
     * @param callbacks Callbacks to business logic
     */
    void setControllerCallbacks(const ControllerCallbacks& callbacks);

    /**
     * @brief Run the login view
     * @return Result of the interaction
     */
    Result run();

    /**
     * @brief Show error message
     * @param message Error message to display
     */
    void showError(const std::string& message);

    /**
     * @brief Show success message
     * @param message Success message to display
     */
    void showSuccess(const std::string& message);

    /**
     * @brief Clear all messages
     */
    void clearMessages();

    /**
     * @brief Switch to login mode
     */
    void switchToLogin();

    /**
     * @brief Switch to verification mode
     */
    void switchToVerification();

    /**
     * @brief Set the result and exit
     * @param result Result to set
     */
    void exitWithResult(Result result);

    /**
     * @brief Set initial mode
     * @param mode Mode to start with
     */
    void setMode(Mode mode) { current_mode = mode; }

private:
    // UI State (no business logic)
    Mode current_mode = Mode::LOGIN;
    Result result = Result::CANCELLED;

    // Form data (just UI state)
    std::string username_input;
    std::string password_input;
    bool remember_me = false;

    std::string reg_username_input;
    std::string reg_email_input;
    std::string reg_password_input;
    std::string reg_confirm_password_input;

    std::string reset_email_input;
    std::string reset_token_input;
    std::string reset_new_password_input;

    std::string verify_token_input;

    // Message state
    std::string status_message;
    std::string error_message;
    bool show_error = false;
    bool show_success = false;

    // UI Components
    ftxui::Component container;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    // Controller callbacks
    ControllerCallbacks controller_callbacks;

    // === UI Creation Methods (pure UI, minimal logic) ===

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
     * @brief Clear all form fields
     */
    void clearForms();

    /**
     * @brief Handle login form submission (just data collection)
     */
    void submitLogin();

    /**
     * @brief Handle registration form submission (just data collection)
     */
    void submitRegistration();
};

} // namespace ui