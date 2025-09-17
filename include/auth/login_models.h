#pragma once
#include <string>
#include <optional>

namespace auth {

/**
 * @brief Data structures for login operations (pure data, easily testable)
 */
struct LoginCredentials {
    std::string username;
    std::string password;
    bool remember_me = false;
};

struct RegistrationData {
    std::string username;
    std::string email;
    std::string password;
    std::string confirm_password;
};

struct PasswordResetRequest {
    std::string email;
    std::string token;
    std::string new_password;
};

struct LoginResult {
    bool success = false;
    std::string error_message;
    std::optional<int> user_id;
    std::optional<std::string> session_token;
    std::optional<std::string> refresh_token;
};

struct RegistrationResult {
    bool success = false;
    std::string error_message;
    std::optional<int> user_id;
    std::optional<std::string> verification_token;  // If email verification required
};

} // namespace auth