#include "auth/validation_service.h"
#include <regex>
#include <optional>

namespace auth {

std::optional<std::string> ValidationService::validateEmail(const std::string& email) const {
    if (email.empty()) {
        return "Email cannot be empty";
    }

    if (!isValidEmailFormat(email)) {
        return "Invalid email format";
    }

    return std::nullopt;
}

std::optional<std::string> ValidationService::validatePassword(const std::string& password) const {
    if (password.empty()) {
        return "Password cannot be empty";
    }

    if (password.length() < 6) {
        return "Password must be at least 6 characters";
    }

    // Add more password complexity rules as needed
    return std::nullopt;
}

std::optional<std::string> ValidationService::validateUsername(const std::string& username) const {
    if (username.empty()) {
        return "Username cannot be empty";
    }

    if (username.length() < 3) {
        return "Username must be at least 3 characters";
    }

    if (username.length() > 20) {
        return "Username must be less than 20 characters";
    }

    // Check for valid characters (alphanumeric + underscore)
    std::regex valid_pattern("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(username, valid_pattern)) {
        return "Username can only contain letters, numbers, and underscores";
    }

    return std::nullopt;
}

std::optional<std::string> ValidationService::validatePasswordConfirmation(
    const std::string& password,
    const std::string& confirm_password) const {

    if (password != confirm_password) {
        return "Passwords do not match";
    }

    return std::nullopt;
}

std::optional<std::string> ValidationService::validateLoginCredentials(
    const std::string& username,
    const std::string& password) const {

    if (username.empty() || password.empty()) {
        return "Please enter username and password";
    }

    return std::nullopt;
}

std::optional<std::string> ValidationService::validateRegistrationData(
    const std::string& username,
    const std::string& email,
    const std::string& password,
    const std::string& confirm_password) const {

    if (username.empty() || email.empty() || password.empty() || confirm_password.empty()) {
        return "Please fill in all fields";
    }

    // Check username
    if (auto error = validateUsername(username)) {
        return error;
    }

    // Check email
    if (auto error = validateEmail(email)) {
        return error;
    }

    // Check password
    if (auto error = validatePassword(password)) {
        return error;
    }

    // Check password confirmation
    if (auto error = validatePasswordConfirmation(password, confirm_password)) {
        return error;
    }

    return std::nullopt;
}

bool ValidationService::isValidEmailFormat(const std::string& email) const {
    // Basic email validation regex
    std::regex email_pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(email, email_pattern);
}

} // namespace auth