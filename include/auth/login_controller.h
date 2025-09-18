#pragma once
#include "auth/login_models.h"
#include "auth/validation_service.h"
#include <memory>
#include <functional>

namespace auth {
    class AuthenticationService;
}

namespace auth {

/**
 * @brief Pure business logic for login operations (no UI dependencies)
 * This class is easily testable with mocks
 */
class LoginController {
public:
    /**
     * @brief Interface for communicating back to the UI
     * This allows the controller to be UI-framework agnostic
     */
    struct ViewCallbacks {
        std::function<void(const std::string&)> showError;
        std::function<void(const std::string&)> showSuccess;
        std::function<void()> clearMessages;
        std::function<void()> switchToLogin;
        std::function<void()> switchToVerification;
        std::function<void(int user_id, const std::string& token)> onLoginSuccess;
    };

    /**
     * @brief Constructor
     * @param auth_service Authentication service dependency
     * @param validation_service Validation service dependency
     */
    explicit LoginController(
        AuthenticationService& auth_service,
        std::unique_ptr<ValidationService> validation_service = std::make_unique<ValidationService>()
    );

    /**
     * @brief Set the view callbacks for communication with UI
     * @param callbacks Callback functions for UI operations
     */
    void setViewCallbacks(const ViewCallbacks& callbacks);

    /**
     * @brief Handle login attempt
     * @param credentials Login credentials
     */
    void handleLogin(const LoginCredentials& credentials);

    /**
     * @brief Handle registration attempt
     * @param data Registration data
     */
    void handleRegistration(const RegistrationData& data);

    /**
     * @brief Handle password reset request
     * @param email Email for password reset
     */
    void handlePasswordResetRequest(const std::string& email);

    /**
     * @brief Handle password reset with token
     * @param token Reset token
     * @param new_password New password
     */
    void handlePasswordReset(const std::string& token, const std::string& new_password);

    /**
     * @brief Handle email verification
     * @param token Verification token
     */
    void handleEmailVerification(const std::string& token);

    /**
     * @brief Get last login result (for testing)
     * @return Last login result
     */
    const LoginResult& getLastLoginResult() const { return last_login_result; }

    /**
     * @brief Get last registration result (for testing)
     * @return Last registration result
     */
    const RegistrationResult& getLastRegistrationResult() const { return last_registration_result; }

private:
    AuthenticationService& auth_service;
    std::unique_ptr<ValidationService> validation_service;
    ViewCallbacks view_callbacks;

    // State for testing
    LoginResult last_login_result;
    RegistrationResult last_registration_result;

    void notifyError(const std::string& message);
    void notifySuccess(const std::string& message);
};

} // namespace auth