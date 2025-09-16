#include "auth/authentication_service.h"
#include "db/player_repository.h"
#include "db/database_manager.h"
#include "log.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <openssl/sha.h>
#include <openssl/rand.h>

namespace auth {

// === Constructor ===

AuthenticationService::AuthenticationService(
    db::PlayerRepository& player_repo,
    db::DatabaseManager& db_manager
) : player_repo(player_repo), db_manager(db_manager), rng(std::random_device{}()) {
    // Load configuration from environment if available
    const char* session_hours = std::getenv("AUTH_SESSION_LIFETIME_HOURS");
    if (session_hours) {
        session_lifetime_hours = std::stoi(session_hours);
    }

    const char* refresh_days = std::getenv("AUTH_REFRESH_LIFETIME_DAYS");
    if (refresh_days) {
        refresh_lifetime_days = std::stoi(refresh_days);
    }

    const char* max_attempts = std::getenv("AUTH_MAX_LOGIN_ATTEMPTS");
    if (max_attempts) {
        max_login_attempts = std::stoi(max_attempts);
    }

    const char* lockout_mins = std::getenv("AUTH_LOCKOUT_DURATION_MINUTES");
    if (lockout_mins) {
        lockout_duration_minutes = std::stoi(lockout_mins);
    }

    // Load password requirements from environment
    const char* min_pass_len = std::getenv("AUTH_MIN_PASSWORD_LENGTH");
    if (min_pass_len) {
        min_password_length = std::stoi(min_pass_len);
    }

    const char* req_upper = std::getenv("AUTH_REQUIRE_UPPERCASE");
    if (req_upper) {
        require_uppercase = std::string(req_upper) == "true";
    }

    const char* req_lower = std::getenv("AUTH_REQUIRE_LOWERCASE");
    if (req_lower) {
        require_lowercase = std::string(req_lower) == "true";
    }

    const char* req_nums = std::getenv("AUTH_REQUIRE_NUMBERS");
    if (req_nums) {
        require_numbers = std::string(req_nums) == "true";
    }

    const char* req_syms = std::getenv("AUTH_REQUIRE_SYMBOLS");
    if (req_syms) {
        require_symbols = std::string(req_syms) == "true";
    }
}

// === Registration ===

RegistrationResult AuthenticationService::registerUser(
    const std::string& username,
    const std::string& email,
    const std::string& password
) {
    RegistrationResult result;

    // Validate inputs
    auto username_error = validateUsername(username);
    if (username_error.has_value()) {
        result.error_message = username_error.value();
        return result;
    }

    auto email_error = validateEmail(email);
    if (email_error.has_value()) {
        result.error_message = email_error.value();
        return result;
    }

    auto password_error = validatePassword(password);
    if (password_error.has_value()) {
        result.error_message = password_error.value();
        return result;
    }

    // Check if username already exists
    auto existing_user = player_repo.findUserByUsername(username);
    if (existing_user.has_value()) {
        result.error_message = "Username already taken";
        return result;
    }

    // Check if email already exists
    existing_user = player_repo.findUserByEmail(email);
    if (existing_user.has_value()) {
        result.error_message = "Email already registered";
        return result;
    }

    // Generate salt and hash password
    std::string salt = generateSalt();
    std::string password_hash = hashPassword(password, salt);

    // Create user
    auto user_id = player_repo.createUser(username, email, password_hash, salt);
    if (!user_id.has_value()) {
        result.error_message = "Failed to create user account";
        return result;
    }

    // Generate email verification token
    std::string verification_token = generateToken(32);
    auto expires_at = std::chrono::system_clock::now() + std::chrono::hours(24);

    if (player_repo.createEmailVerificationToken(user_id.value(), verification_token, expires_at)) {
        result.verification_token = verification_token;
    }

    result.success = true;
    result.user_id = user_id.value();

    Log::info("User registered successfully: " + username);
    return result;
}

bool AuthenticationService::verifyEmail(const std::string& token) {
    auto user_id = player_repo.validateEmailVerificationToken(token);
    if (!user_id.has_value()) {
        return false;
    }

    // Mark email as verified
    if (!player_repo.markEmailVerified(user_id.value())) {
        return false;
    }

    // Mark token as used
    return player_repo.markEmailVerificationTokenUsed(token);
}

// === Authentication ===

LoginResult AuthenticationService::login(
    const std::string& username,
    const std::string& password,
    bool remember_me,
    const std::string& ip_address,
    const std::string& user_agent
) {
    LoginResult result;

    // Find user by username or email
    auto user = player_repo.findUserByUsername(username);
    if (!user.has_value()) {
        user = player_repo.findUserByEmail(username);
        if (!user.has_value()) {
            result.error_message = "Invalid username or password";
            return result;
        }
    }

    // Check if account is locked
    if (isAccountLocked(user.value())) {
        result.error_message = "Account is locked due to too many failed login attempts";

        // Record failed attempt
        player_repo.recordLoginAttempt(
            user->id, false, ip_address, user_agent, "Account locked"
        );

        return result;
    }

    // Verify password
    if (!verifyPassword(password, user->password_hash, user->salt)) {
        // Increment failed login attempts
        int failed_attempts = player_repo.incrementFailedLogins(user->id);

        // Record failed attempt
        player_repo.recordLoginAttempt(
            user->id, false, ip_address, user_agent, "Invalid password"
        );

        // Check if we should lock the account
        if (failed_attempts >= max_login_attempts) {
            player_repo.setAccountLocked(user->id, true);
            result.error_message = "Account locked due to too many failed login attempts";
        } else {
            result.error_message = "Invalid username or password";
        }

        return result;
    }

    // Check if email is verified (optional - you can make this required)
    if (!user->email_verified) {
        Log::warn("User logging in with unverified email: " + user->username);
    }

    // Reset failed login attempts on successful login
    player_repo.resetFailedLogins(user->id);

    // Generate session tokens
    std::string session_token = generateToken(32);
    std::string refresh_token = generateToken(32);

    // Calculate expiry times
    auto session_expires = calculateSessionExpiry(remember_me);
    auto refresh_expires = calculateRefreshExpiry();

    // Create session
    db::UserSession session;
    session.user_id = user->id;
    session.session_token = session_token;
    session.refresh_token = refresh_token;
    session.expires_at = session_expires;
    session.refresh_expires_at = refresh_expires;
    session.ip_address = ip_address;
    session.user_agent = user_agent;
    session.remember_me = remember_me;

    auto session_id = player_repo.createSession(session);
    if (!session_id.has_value()) {
        result.error_message = "Failed to create session";
        return result;
    }

    // Update last login time
    player_repo.updateLastLogin(user->id);

    // Record successful login
    player_repo.recordLoginAttempt(
        user->id, true, ip_address, user_agent, "", session_id
    );

    result.success = true;
    result.session_token = session_token;
    result.refresh_token = refresh_token;
    result.user_id = user->id;

    Log::info("User logged in successfully: " + user->username);
    return result;
}

bool AuthenticationService::logout(const std::string& session_token) {
    auto session = player_repo.findSessionByToken(session_token);
    if (!session.has_value()) {
        return false;
    }

    return player_repo.revokeSession(session->id);
}

int AuthenticationService::logoutAllSessions(int user_id) {
    return player_repo.revokeAllUserSessions(user_id);
}

// === Session Management ===

SessionValidation AuthenticationService::validateSession(const std::string& session_token) {
    SessionValidation result;

    auto session = player_repo.findSessionByToken(session_token);
    if (!session.has_value()) {
        result.error_message = "Invalid session token";
        return result;
    }

    // Check if session is expired
    auto now = std::chrono::system_clock::now();
    if (now > session->expires_at) {
        result.error_message = "Session expired";
        result.needs_refresh = true;
        return result;
    }

    // Check if session is close to expiry (within 1 hour)
    auto one_hour_before_expiry = session->expires_at - std::chrono::hours(1);
    if (now > one_hour_before_expiry) {
        result.needs_refresh = true;
    }

    // Update last used time
    player_repo.updateSessionLastUsed(session->id);

    result.valid = true;
    result.user_id = session->user_id;
    return result;
}

LoginResult AuthenticationService::refreshSession(
    const std::string& refresh_token,
    const std::string& ip_address,
    const std::string& user_agent
) {
    LoginResult result;

    // Find session by refresh token
    // Note: This requires extending PlayerRepository to search by refresh token
    // For now, we'll need to add this functionality
    // This is a simplified version - you'd need to extend the repository

    // Temporarily mark parameters as used until implementation is complete
    (void)refresh_token;
    (void)ip_address;
    (void)user_agent;

    result.error_message = "Refresh not yet implemented";
    return result;
}

int AuthenticationService::cleanupExpiredSessions() {
    return player_repo.cleanupExpiredSessions();
}

// === Password Management ===

bool AuthenticationService::changePassword(
    int user_id,
    const std::string& old_password,
    const std::string& new_password
) {
    // Get user
    auto user = player_repo.findUserById(user_id);
    if (!user.has_value()) {
        return false;
    }

    // Verify old password
    if (!verifyPassword(old_password, user->password_hash, user->salt)) {
        return false;
    }

    // Validate new password
    auto password_error = validatePassword(new_password);
    if (password_error.has_value()) {
        return false;
    }

    // Generate new salt and hash
    std::string new_salt = generateSalt();
    std::string new_hash = hashPassword(new_password, new_salt);

    // Update password
    bool success = player_repo.updatePassword(user_id, new_hash, new_salt);

    if (success) {
        // Optionally revoke all sessions to force re-login
        player_repo.revokeAllUserSessions(user_id);
        Log::info("Password changed for user ID: " + std::to_string(user_id));
    }

    return success;
}

std::optional<std::string> AuthenticationService::requestPasswordReset(const std::string& email) {
    // Find user by email
    auto user = player_repo.findUserByEmail(email);
    if (!user.has_value()) {
        // Don't reveal whether email exists
        return std::nullopt;
    }

    // Generate reset token
    std::string reset_token = generateToken(32);
    auto expires_at = std::chrono::system_clock::now() + std::chrono::hours(1);

    // Store token
    if (!player_repo.createPasswordResetToken(user->id, reset_token, expires_at)) {
        return std::nullopt;
    }

    Log::info("Password reset requested for: " + user->username);
    return reset_token;
}

bool AuthenticationService::resetPassword(const std::string& token, const std::string& new_password) {
    // Validate token
    auto user_id = player_repo.validatePasswordResetToken(token);
    if (!user_id.has_value()) {
        return false;
    }

    // Validate new password
    auto password_error = validatePassword(new_password);
    if (password_error.has_value()) {
        return false;
    }

    // Generate new salt and hash
    std::string new_salt = generateSalt();
    std::string new_hash = hashPassword(new_password, new_salt);

    // Update password
    if (!player_repo.updatePassword(user_id.value(), new_hash, new_salt)) {
        return false;
    }

    // Mark token as used
    player_repo.markPasswordResetTokenUsed(token);

    // Revoke all sessions
    player_repo.revokeAllUserSessions(user_id.value());

    Log::info("Password reset completed for user ID: " + std::to_string(user_id.value()));
    return true;
}

// === Validation ===

std::optional<std::string> AuthenticationService::validateUsername(const std::string& username) {
    if (username.length() < 3) {
        return "Username must be at least 3 characters long";
    }

    if (username.length() > 50) {
        return "Username must be no more than 50 characters";
    }

    if (!isValidUsernameFormat(username)) {
        return "Username can only contain letters, numbers, and underscores";
    }

    return std::nullopt;
}

std::optional<std::string> AuthenticationService::validateEmail(const std::string& email) {
    if (email.empty()) {
        return "Email is required";
    }

    if (email.length() > 255) {
        return "Email must be no more than 255 characters";
    }

    if (!isValidEmailFormat(email)) {
        return "Invalid email format";
    }

    return std::nullopt;
}

std::optional<std::string> AuthenticationService::validatePassword(const std::string& password) {
    if (password.length() < min_password_length) {
        return "Password must be at least " + std::to_string(min_password_length) + " characters long";
    }

    if (!isPasswordStrong(password)) {
        std::string requirements = "Password must contain";
        std::vector<std::string> reqs;

        if (require_uppercase) reqs.push_back("uppercase letters");
        if (require_lowercase) reqs.push_back("lowercase letters");
        if (require_numbers) reqs.push_back("numbers");
        if (require_symbols) reqs.push_back("symbols");

        for (size_t i = 0; i < reqs.size(); ++i) {
            if (i == 0) requirements += " ";
            else if (i == reqs.size() - 1) requirements += " and ";
            else requirements += ", ";
            requirements += reqs[i];
        }

        return requirements;
    }

    return std::nullopt;
}

// === Helper Functions ===

std::string AuthenticationService::generateSalt() {
    const size_t salt_length = 16;
    unsigned char salt_bytes[salt_length];

    // Generate random bytes
    if (RAND_bytes(salt_bytes, salt_length) != 1) {
        // Fallback to less secure random if OpenSSL fails
        std::uniform_int_distribution<int> dist(0, 255);
        for (size_t i = 0; i < salt_length; ++i) {
            salt_bytes[i] = static_cast<unsigned char>(dist(rng));
        }
    }

    // Convert to hex string
    std::stringstream ss;
    for (size_t i = 0; i < salt_length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt_bytes[i];
    }

    return ss.str();
}

std::string AuthenticationService::hashPassword(const std::string& password, const std::string& salt) {
    // Combine password and salt
    std::string salted = password + salt;

    // SHA-256 hash
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted.c_str()), salted.length(), hash);

    // Convert to hex string
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

bool AuthenticationService::verifyPassword(
    const std::string& password,
    const std::string& hash,
    const std::string& salt
) {
    std::string computed_hash = hashPassword(password, salt);
    return computed_hash == hash;
}

std::string AuthenticationService::generateToken(size_t length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);

    std::string token;
    token.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        token += chars[dist(rng)];
    }

    return token;
}

bool AuthenticationService::isAccountLocked(const db::User& user) const {
    if (!user.account_locked) {
        return false;
    }

    // Check if lockout period has expired
    if (user.last_failed_login.time_since_epoch().count() > 0) {
        auto lockout_expires = user.last_failed_login + std::chrono::minutes(lockout_duration_minutes);
        if (std::chrono::system_clock::now() > lockout_expires) {
            // Lockout period has expired
            // Note: You'd need to add a method to unlock the account here
            return false;
        }
    }

    return true;
}

std::chrono::system_clock::time_point AuthenticationService::calculateSessionExpiry(bool remember_me) const {
    if (remember_me) {
        // Remember-me sessions last as long as refresh tokens
        return std::chrono::system_clock::now() + std::chrono::hours(24 * refresh_lifetime_days);
    } else {
        return std::chrono::system_clock::now() + std::chrono::hours(session_lifetime_hours);
    }
}

std::chrono::system_clock::time_point AuthenticationService::calculateRefreshExpiry() const {
    return std::chrono::system_clock::now() + std::chrono::hours(24 * refresh_lifetime_days);
}

bool AuthenticationService::isValidEmailFormat(const std::string& email) const {
    const std::regex email_regex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
    );
    return std::regex_match(email, email_regex);
}

bool AuthenticationService::isValidUsernameFormat(const std::string& username) const {
    return std::all_of(username.begin(), username.end(), [](char c) {
        return std::isalnum(c) || c == '_';
    });
}

bool AuthenticationService::isPasswordStrong(const std::string& password) const {
    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_symbol = false;

    for (char c : password) {
        if (std::isupper(c)) has_upper = true;
        if (std::islower(c)) has_lower = true;
        if (std::isdigit(c)) has_digit = true;
        if (!std::isalnum(c)) has_symbol = true;
    }

    if (require_uppercase && !has_upper) return false;
    if (require_lowercase && !has_lower) return false;
    if (require_numbers && !has_digit) return false;
    if (require_symbols && !has_symbol) return false;

    return true;
}

} // namespace auth