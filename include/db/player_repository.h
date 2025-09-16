#pragma once

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <chrono>

namespace db {

// Forward declarations
class DatabaseManager;

/**
 * @struct User
 * @brief Represents a user account in the system
 */
struct User {
    int id = 0;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string salt;
    bool email_verified = false;
    bool account_locked = false;
    int failed_login_attempts = 0;
    std::chrono::system_clock::time_point last_failed_login;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::optional<std::chrono::system_clock::time_point> last_login;
};

/**
 * @struct UserProfile
 * @brief Extended user profile information
 */
struct UserProfile {
    int id = 0;
    int user_id = 0;
    std::optional<std::string> display_name;
    std::optional<std::string> avatar_url;
    std::string timezone = "UTC";
    std::string language = "en";
    std::string theme = "auto";
    std::string privacy_settings = "{}";  // JSON
    std::string game_settings = "{}";     // JSON
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

/**
 * @struct UserSession
 * @brief Represents an active user session
 */
struct UserSession {
    int id = 0;
    int user_id = 0;
    std::string session_token;
    std::optional<std::string> refresh_token;
    std::chrono::system_clock::time_point expires_at;
    std::optional<std::chrono::system_clock::time_point> refresh_expires_at;
    std::optional<std::string> ip_address;
    std::optional<std::string> user_agent;
    bool remember_me = false;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_used;
    bool revoked = false;
    std::optional<std::chrono::system_clock::time_point> revoked_at;
};

/**
 * @class PlayerRepository
 * @brief Repository pattern for user/player database operations
 */
class PlayerRepository {
public:
    explicit PlayerRepository(DatabaseManager& db_manager);
    ~PlayerRepository() = default;

    // === User Management ===

    /**
     * @brief Create a new user account
     * @param username Unique username
     * @param email User's email address
     * @param password_hash Hashed password
     * @param salt Password salt
     * @return Created user ID if successful
     */
    std::optional<int> createUser(
        const std::string& username,
        const std::string& email,
        const std::string& password_hash,
        const std::string& salt
    );

    /**
     * @brief Find user by username
     * @param username Username to search for
     * @return User object if found
     */
    std::optional<User> findUserByUsername(const std::string& username);

    /**
     * @brief Find user by email
     * @param email Email to search for
     * @return User object if found
     */
    std::optional<User> findUserByEmail(const std::string& email);

    /**
     * @brief Find user by ID
     * @param user_id User ID
     * @return User object if found
     */
    std::optional<User> findUserById(int user_id);

    /**
     * @brief Update user's password
     * @param user_id User ID
     * @param new_password_hash New password hash
     * @param new_salt New salt
     * @return Success status
     */
    bool updatePassword(int user_id, const std::string& new_password_hash, const std::string& new_salt);

    /**
     * @brief Update last login time
     * @param user_id User ID
     * @return Success status
     */
    bool updateLastLogin(int user_id);

    /**
     * @brief Increment failed login attempts
     * @param user_id User ID
     * @return New failed attempt count
     */
    int incrementFailedLogins(int user_id);

    /**
     * @brief Reset failed login attempts
     * @param user_id User ID
     * @return Success status
     */
    bool resetFailedLogins(int user_id);

    /**
     * @brief Lock or unlock user account
     * @param user_id User ID
     * @param locked Lock status
     * @return Success status
     */
    bool setAccountLocked(int user_id, bool locked);

    /**
     * @brief Mark email as verified
     * @param user_id User ID
     * @return Success status
     */
    bool markEmailVerified(int user_id);

    // === Profile Management ===

    /**
     * @brief Get user profile
     * @param user_id User ID
     * @return UserProfile if exists
     */
    std::optional<UserProfile> getUserProfile(int user_id);

    /**
     * @brief Create or update user profile
     * @param profile Profile data
     * @return Success status
     */
    bool upsertUserProfile(const UserProfile& profile);

    // === Session Management ===

    /**
     * @brief Create a new session
     * @param session Session data
     * @return Session ID if successful
     */
    std::optional<int> createSession(const UserSession& session);

    /**
     * @brief Find session by token
     * @param token Session token
     * @return Session if found and valid
     */
    std::optional<UserSession> findSessionByToken(const std::string& token);

    /**
     * @brief Update session last used time
     * @param session_id Session ID
     * @return Success status
     */
    bool updateSessionLastUsed(int session_id);

    /**
     * @brief Revoke a session
     * @param session_id Session ID
     * @return Success status
     */
    bool revokeSession(int session_id);

    /**
     * @brief Revoke all user sessions
     * @param user_id User ID
     * @return Number of sessions revoked
     */
    int revokeAllUserSessions(int user_id);

    /**
     * @brief Clean up expired sessions
     * @return Number of sessions deleted
     */
    int cleanupExpiredSessions();

    // === Token Management ===

    /**
     * @brief Create password reset token
     * @param user_id User ID
     * @param token Token string
     * @param expires_at Expiration time
     * @return Success status
     */
    bool createPasswordResetToken(
        int user_id,
        const std::string& token,
        const std::chrono::system_clock::time_point& expires_at
    );

    /**
     * @brief Validate password reset token
     * @param token Token to validate
     * @return User ID if valid
     */
    std::optional<int> validatePasswordResetToken(const std::string& token);

    /**
     * @brief Mark password reset token as used
     * @param token Token string
     * @return Success status
     */
    bool markPasswordResetTokenUsed(const std::string& token);

    /**
     * @brief Create email verification token
     * @param user_id User ID
     * @param token Token string
     * @param expires_at Expiration time
     * @return Success status
     */
    bool createEmailVerificationToken(
        int user_id,
        const std::string& token,
        const std::chrono::system_clock::time_point& expires_at
    );

    /**
     * @brief Validate email verification token
     * @param token Token to validate
     * @return User ID if valid
     */
    std::optional<int> validateEmailVerificationToken(const std::string& token);

    /**
     * @brief Mark email verification token as used
     * @param token Token string
     * @return Success status
     */
    bool markEmailVerificationTokenUsed(const std::string& token);

    // === Login History ===

    /**
     * @brief Record login attempt
     * @param user_id User ID
     * @param success Success status
     * @param ip_address IP address
     * @param user_agent User agent string
     * @param failure_reason Optional failure reason
     * @param session_id Optional session ID if successful
     * @return Success status
     */
    bool recordLoginAttempt(
        int user_id,
        bool success,
        const std::string& ip_address,
        const std::string& user_agent,
        const std::string& failure_reason = "",
        std::optional<int> session_id = std::nullopt
    );

    /**
     * @brief Get recent login history
     * @param user_id User ID
     * @param limit Number of records to return
     * @return Vector of login records (simplified structure)
     */
    std::vector<std::pair<std::chrono::system_clock::time_point, bool>>
        getRecentLoginHistory(int user_id, int limit = 10);

private:
    DatabaseManager& db_manager;

    // Helper functions for timestamp conversion
    std::string timestampToString(const std::chrono::system_clock::time_point& time) const;
    std::chrono::system_clock::time_point stringToTimestamp(const std::string& str) const;
};

} // namespace db