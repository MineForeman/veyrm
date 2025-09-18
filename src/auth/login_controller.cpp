#include "auth/login_controller.h"
#include "auth/authentication_service.h"
#include "log.h"

namespace auth {

LoginController::LoginController(
    AuthenticationService& auth_service,
    std::unique_ptr<ValidationService> validation_service)
    : auth_service(auth_service)
    , validation_service(std::move(validation_service)) {
}

void LoginController::setViewCallbacks(const ViewCallbacks& callbacks) {
    view_callbacks = callbacks;
}

void LoginController::handleLogin(const LoginCredentials& credentials) {
    // Clear any previous messages
    if (view_callbacks.clearMessages) {
        view_callbacks.clearMessages();
    }

    // Validate input
    if (auto error = validation_service->validateLoginCredentials(credentials.username, credentials.password)) {
        notifyError(*error);
        return;
    }

    // Attempt login
    try {
        last_login_result = auth_service.login(
            credentials.username,
            credentials.password,
            credentials.remember_me,
            "127.0.0.1",  // Could be passed in as parameter
            "Veyrm-Client"
        );

        if (last_login_result.success) {
            Log::info("User logged in successfully: " + credentials.username);

            if (view_callbacks.onLoginSuccess && last_login_result.user_id && last_login_result.session_token) {
                view_callbacks.onLoginSuccess(*last_login_result.user_id, *last_login_result.session_token);
            }
        } else {
            notifyError(last_login_result.error_message);
        }
    } catch (const std::exception& e) {
        Log::error("Login error: " + std::string(e.what()));
        notifyError("Login failed due to system error");
    }
}

void LoginController::handleRegistration(const RegistrationData& data) {
    // Clear any previous messages
    if (view_callbacks.clearMessages) {
        view_callbacks.clearMessages();
    }

    // Validate input
    if (auto error = validation_service->validateRegistrationData(
        data.username, data.email, data.password, data.confirm_password)) {
        notifyError(*error);
        return;
    }

    // Attempt registration
    try {
        last_registration_result = auth_service.registerUser(
            data.username,
            data.email,
            data.password
        );

        if (last_registration_result.success) {
            Log::info("User registered successfully: " + data.username);

            if (last_registration_result.verification_token && !last_registration_result.verification_token->empty()) {
                // Email verification required
                notifySuccess("Registration successful! Please check your email for verification.");
                if (view_callbacks.switchToVerification) {
                    view_callbacks.switchToVerification();
                }
            } else {
                // Registration complete
                notifySuccess("Registration successful! You can now log in.");
                if (view_callbacks.switchToLogin) {
                    view_callbacks.switchToLogin();
                }
            }
        } else {
            notifyError(last_registration_result.error_message);
        }
    } catch (const std::exception& e) {
        Log::error("Registration error: " + std::string(e.what()));
        notifyError("Registration failed due to system error");
    }
}

void LoginController::handlePasswordResetRequest(const std::string& email) {
    // Clear any previous messages
    if (view_callbacks.clearMessages) {
        view_callbacks.clearMessages();
    }

    // Validate email
    if (auto error = validation_service->validateEmail(email)) {
        notifyError(*error);
        return;
    }

    // Request password reset
    try {
        auto token = auth_service.requestPasswordReset(email);
        if (token.has_value()) {
            notifySuccess("Password reset instructions sent to your email.");
        } else {
            notifyError("Failed to send password reset email. Please check the email address.");
        }
    } catch (const std::exception& e) {
        Log::error("Password reset request error: " + std::string(e.what()));
        notifyError("Password reset request failed due to system error");
    }
}

void LoginController::handlePasswordReset(const std::string& token, const std::string& new_password) {
    // Clear any previous messages
    if (view_callbacks.clearMessages) {
        view_callbacks.clearMessages();
    }

    // Validate inputs
    if (token.empty()) {
        notifyError("Please enter reset token");
        return;
    }

    if (auto error = validation_service->validatePassword(new_password)) {
        notifyError(*error);
        return;
    }

    // Reset password
    try {
        bool success = auth_service.resetPassword(token, new_password);
        if (success) {
            notifySuccess("Password reset successfully! You can now log in with your new password.");
            if (view_callbacks.switchToLogin) {
                view_callbacks.switchToLogin();
            }
        } else {
            notifyError("Invalid or expired reset token");
        }
    } catch (const std::exception& e) {
        Log::error("Password reset error: " + std::string(e.what()));
        notifyError("Password reset failed due to system error");
    }
}

void LoginController::handleEmailVerification(const std::string& token) {
    // Clear any previous messages
    if (view_callbacks.clearMessages) {
        view_callbacks.clearMessages();
    }

    // Validate token
    if (token.empty()) {
        notifyError("Please enter verification token");
        return;
    }

    // Verify email
    try {
        bool success = auth_service.verifyEmail(token);
        if (success) {
            notifySuccess("Email verified successfully! You can now log in.");
            if (view_callbacks.switchToLogin) {
                view_callbacks.switchToLogin();
            }
        } else {
            notifyError("Invalid or expired verification token");
        }
    } catch (const std::exception& e) {
        Log::error("Email verification error: " + std::string(e.what()));
        notifyError("Email verification failed due to system error");
    }
}

void LoginController::notifyError(const std::string& message) {
    if (view_callbacks.showError) {
        view_callbacks.showError(message);
    }
}

void LoginController::notifySuccess(const std::string& message) {
    if (view_callbacks.showSuccess) {
        view_callbacks.showSuccess(message);
    }
}

} // namespace auth