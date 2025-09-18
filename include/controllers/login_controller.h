/**
 * @file login_controller.h
 * @brief Controller for login/authentication business logic
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <memory>
#include <functional>

// Forward declarations
namespace auth {
    class AuthenticationService;
    class ValidationService;
}

namespace controllers {

/**
 * @class LoginController
 * @brief Handles authentication and user registration logic
 */
class LoginController {
public:
    /**
     * @brief Authentication mode
     */
    enum class Mode {
        LOGIN,
        REGISTER,
        FORGOT_PASSWORD,
        VERIFY_EMAIL
    };

    /**
     * @brief Authentication result
     */
    struct AuthResult {
        bool success;
        int user_id;
        std::string session_token;
        std::string error_message;
    };

    /**
     * @brief Callbacks for view layer communication
     */
    struct ViewCallbacks {
        std::function<void(const std::string&)> showMessage;
        std::function<void(const std::string&)> showError;
        std::function<void(float)> updateProgress;
        std::function<void()> onSuccess;
        std::function<void()> onCancel;
    };

    /**
     * @brief Constructor
     * @param auth_service Authentication service
     */
    explicit LoginController(auth::AuthenticationService& auth_service);

    ~LoginController();

    /**
     * @brief Set view callbacks
     * @param callbacks View callback functions
     */
    void setViewCallbacks(const ViewCallbacks& callbacks);

    /**
     * @brief Set current mode
     * @param mode Authentication mode
     */
    void setMode(Mode mode) { current_mode = mode; }

    /**
     * @brief Get current mode
     * @return Current authentication mode
     */
    Mode getMode() const { return current_mode; }

    /**
     * @brief Attempt login
     * @param username Username or email
     * @param password Password
     * @return Authentication result
     */
    AuthResult login(const std::string& username, const std::string& password);

    /**
     * @brief Register new user
     * @param username Username
     * @param email Email address
     * @param password Password
     * @param confirm_password Password confirmation
     * @return Authentication result
     */
    AuthResult registerUser(const std::string& username,
                          const std::string& email,
                          const std::string& password,
                          const std::string& confirm_password);

    /**
     * @brief Request password reset
     * @param email Email address
     * @return Success status
     */
    bool requestPasswordReset(const std::string& email);

    /**
     * @brief Verify email with code
     * @param email Email address
     * @param code Verification code
     * @return Success status
     */
    bool verifyEmail(const std::string& email, const std::string& code);

    /**
     * @brief Validate username format
     * @param username Username to validate
     * @return Error message if invalid, empty if valid
     */
    std::string validateUsername(const std::string& username) const;

    /**
     * @brief Validate email format
     * @param email Email to validate
     * @return Error message if invalid, empty if valid
     */
    std::string validateEmail(const std::string& email) const;

    /**
     * @brief Validate password strength
     * @param password Password to validate
     * @return Error message if invalid, empty if valid
     */
    std::string validatePassword(const std::string& password) const;

    /**
     * @brief Check if passwords match
     * @param password Password
     * @param confirm Confirmation password
     * @return True if passwords match
     */
    bool passwordsMatch(const std::string& password, const std::string& confirm) const;

    /**
     * @brief Get last authentication result
     * @return Last authentication result
     */
    const AuthResult& getLastResult() const { return last_result; }

    /**
     * @brief Get authenticated user ID
     * @return User ID or 0 if not authenticated
     */
    int getUserId() const { return last_result.user_id; }

    /**
     * @brief Get session token
     * @return Session token or empty if not authenticated
     */
    std::string getSessionToken() const { return last_result.session_token; }

private:
    auth::AuthenticationService& auth_service;
    std::unique_ptr<auth::ValidationService> validator;

    Mode current_mode;
    AuthResult last_result;
    ViewCallbacks view_callbacks;

    /**
     * @brief Clear last result
     */
    void clearResult();

    /**
     * @brief Show error through view callback
     * @param error Error message
     */
    void showError(const std::string& error);

    /**
     * @brief Show message through view callback
     * @param message Status message
     */
    void showMessage(const std::string& message);

    /**
     * @brief Update progress through view callback
     * @param progress Progress value (0.0 to 1.0)
     */
    void updateProgress(float progress);
};

} // namespace controllers