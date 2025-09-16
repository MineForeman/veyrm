#pragma once

#include <string>
#include <optional>
#include <memory>
#include <chrono>
#include <random>

namespace db {
    class PlayerRepository;
    class DatabaseManager;
    struct User;
    struct UserSession;
}

namespace auth {

/**
 * @struct LoginResult
 * @brief Result of a login attempt
 */
struct LoginResult {
    bool success = false;
    std::optional<std::string> session_token;
    std::optional<std::string> refresh_token;
    std::string error_message;
    std::optional<int> user_id;
};

/**
 * @struct RegistrationResult
 * @brief Result of a registration attempt
 */
struct RegistrationResult {
    bool success = false;
    std::optional<int> user_id;
    std::string error_message;
    std::optional<std::string> verification_token;
};

/**
 * @struct SessionValidation
 * @brief Result of session validation
 */
struct SessionValidation {
    bool valid = false;
    std::optional<int> user_id;
    bool needs_refresh = false;
    std::string error_message;
};

/**
 * @class AuthenticationService
 * @brief Handles user authentication, registration, and session management
 */
class AuthenticationService {
public:
    /**
     * @brief Constructor
     * @param player_repo Reference to PlayerRepository
     * @param db_manager Reference to DatabaseManager
     */
    AuthenticationService(db::PlayerRepository& player_repo, db::DatabaseManager& db_manager);
    ~AuthenticationService() = default;

    // === Registration ===

    /**
     * @brief Register a new user
     * @param username Desired username
     * @param email User's email address
     * @param password Plain text password
     * @return Registration result
     */
    RegistrationResult registerUser(
        const std::string& username,
        const std::string& email,
        const std::string& password
    );

    /**
     * @brief Verify email address with token
     * @param token Verification token
     * @return Success status
     */
    bool verifyEmail(const std::string& token);

    // === Authentication ===

    /**
     * @brief Authenticate user with username and password
     * @param username Username or email
     * @param password Plain text password
     * @param remember_me Whether to create long-lived session
     * @param ip_address Client IP address
     * @param user_agent Client user agent
     * @return Login result with session tokens
     */
    LoginResult login(
        const std::string& username,
        const std::string& password,
        bool remember_me = false,
        const std::string& ip_address = "",
        const std::string& user_agent = ""
    );

    /**
     * @brief Logout user by invalidating session
     * @param session_token Current session token
     * @return Success status
     */
    bool logout(const std::string& session_token);

    /**
     * @brief Logout all sessions for a user
     * @param user_id User ID
     * @return Number of sessions invalidated
     */
    int logoutAllSessions(int user_id);

    // === Session Management ===

    /**
     * @brief Validate a session token
     * @param session_token Token to validate
     * @return Validation result
     */
    SessionValidation validateSession(const std::string& session_token);

    /**
     * @brief Refresh an expired session using refresh token
     * @param refresh_token Refresh token
     * @param ip_address Client IP address
     * @param user_agent Client user agent
     * @return New login result if successful
     */
    LoginResult refreshSession(
        const std::string& refresh_token,
        const std::string& ip_address = "",
        const std::string& user_agent = ""
    );

    /**
     * @brief Clean up expired sessions
     * @return Number of sessions cleaned
     */
    int cleanupExpiredSessions();

    // === Password Management ===

    /**
     * @brief Change user password
     * @param user_id User ID
     * @param old_password Current password
     * @param new_password New password
     * @return Success status
     */
    bool changePassword(
        int user_id,
        const std::string& old_password,
        const std::string& new_password
    );

    /**
     * @brief Request password reset
     * @param email User's email address
     * @return Reset token if successful
     */
    std::optional<std::string> requestPasswordReset(const std::string& email);

    /**
     * @brief Reset password with token
     * @param token Reset token
     * @param new_password New password
     * @return Success status
     */
    bool resetPassword(const std::string& token, const std::string& new_password);

    // === Validation ===

    /**
     * @brief Validate username format
     * @param username Username to validate
     * @return Error message if invalid
     */
    std::optional<std::string> validateUsername(const std::string& username);

    /**
     * @brief Validate email format
     * @param email Email to validate
     * @return Error message if invalid
     */
    std::optional<std::string> validateEmail(const std::string& email);

    /**
     * @brief Validate password strength
     * @param password Password to validate
     * @return Error message if invalid
     */
    std::optional<std::string> validatePassword(const std::string& password);

    // === Configuration ===

    /**
     * @brief Get session lifetime in hours
     * @return Session lifetime
     */
    int getSessionLifetimeHours() const { return session_lifetime_hours; }

    /**
     * @brief Get refresh token lifetime in days
     * @return Refresh token lifetime
     */
    int getRefreshLifetimeDays() const { return refresh_lifetime_days; }

    /**
     * @brief Get max login attempts before lockout
     * @return Max attempts
     */
    int getMaxLoginAttempts() const { return max_login_attempts; }

    /**
     * @brief Get lockout duration in minutes
     * @return Lockout duration
     */
    int getLockoutDurationMinutes() const { return lockout_duration_minutes; }

    /**
     * @brief Set configuration values
     */
    void setSessionLifetime(int hours) { session_lifetime_hours = hours; }
    void setRefreshLifetime(int days) { refresh_lifetime_days = days; }
    void setMaxLoginAttempts(int attempts) { max_login_attempts = attempts; }
    void setLockoutDuration(int minutes) { lockout_duration_minutes = minutes; }

private:
    db::PlayerRepository& player_repo;
    db::DatabaseManager& db_manager;

    // Configuration
    int session_lifetime_hours = 4;        // Short-lived session tokens
    int refresh_lifetime_days = 30;        // Long-lived refresh tokens
    int max_login_attempts = 5;            // Before account lockout
    int lockout_duration_minutes = 15;     // Lockout duration

    // Password requirements
    size_t min_password_length = 8;
    bool require_uppercase = true;
    bool require_lowercase = true;
    bool require_numbers = true;
    bool require_symbols = false;

    // Random number generator for tokens
    std::mt19937 rng;

    // === Helper Functions ===

    /**
     * @brief Generate random salt
     * @return Salt string
     */
    std::string generateSalt();

    /**
     * @brief Hash password with salt
     * @param password Plain text password
     * @param salt Salt string
     * @return Hashed password
     */
    std::string hashPassword(const std::string& password, const std::string& salt);

    /**
     * @brief Verify password against hash
     * @param password Plain text password
     * @param hash Stored hash
     * @param salt Stored salt
     * @return True if password matches
     */
    bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);

    /**
     * @brief Generate secure random token
     * @param length Token length
     * @return Random token string
     */
    std::string generateToken(size_t length = 32);

    /**
     * @brief Check if account is locked
     * @param user User object
     * @return True if locked
     */
    bool isAccountLocked(const db::User& user) const;

    /**
     * @brief Calculate session expiry time
     * @param remember_me Whether this is a remember-me session
     * @return Expiry time point
     */
    std::chrono::system_clock::time_point calculateSessionExpiry(bool remember_me) const;

    /**
     * @brief Calculate refresh token expiry time
     * @return Expiry time point
     */
    std::chrono::system_clock::time_point calculateRefreshExpiry() const;

    /**
     * @brief Check if email format is valid
     * @param email Email to check
     * @return True if valid
     */
    bool isValidEmailFormat(const std::string& email) const;

    /**
     * @brief Check if username format is valid
     * @param username Username to check
     * @return True if valid
     */
    bool isValidUsernameFormat(const std::string& username) const;

    /**
     * @brief Check password strength
     * @param password Password to check
     * @return True if strong enough
     */
    bool isPasswordStrong(const std::string& password) const;
};

} // namespace auth