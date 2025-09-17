#pragma once
#include <string>
#include <optional>

namespace auth {

/**
 * @brief Pure validation logic (no UI dependencies, 100% testable)
 */
class ValidationService {
public:
    /**
     * @brief Validate email format
     * @param email Email to validate
     * @return Error message if invalid, nullopt if valid
     */
    std::optional<std::string> validateEmail(const std::string& email) const;

    /**
     * @brief Validate password strength
     * @param password Password to validate
     * @return Error message if invalid, nullopt if valid
     */
    std::optional<std::string> validatePassword(const std::string& password) const;

    /**
     * @brief Validate username format
     * @param username Username to validate
     * @return Error message if invalid, nullopt if valid
     */
    std::optional<std::string> validateUsername(const std::string& username) const;

    /**
     * @brief Validate password confirmation
     * @param password Original password
     * @param confirm_password Confirmation password
     * @return Error message if don't match, nullopt if valid
     */
    std::optional<std::string> validatePasswordConfirmation(
        const std::string& password,
        const std::string& confirm_password) const;

    /**
     * @brief Validate login credentials completeness
     * @param username Username
     * @param password Password
     * @return Error message if incomplete, nullopt if valid
     */
    std::optional<std::string> validateLoginCredentials(
        const std::string& username,
        const std::string& password) const;

    /**
     * @brief Validate registration data completeness
     * @param username Username
     * @param email Email
     * @param password Password
     * @param confirm_password Confirmation password
     * @return Error message if invalid, nullopt if valid
     */
    std::optional<std::string> validateRegistrationData(
        const std::string& username,
        const std::string& email,
        const std::string& password,
        const std::string& confirm_password) const;

private:
    bool isValidEmailFormat(const std::string& email) const;
};

} // namespace auth