#include "db/player_repository.h"
#include "db/database_manager.h"
#include "log.h"
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef ENABLE_DATABASE

namespace db {

PlayerRepository::PlayerRepository(DatabaseManager& db_manager)
    : db_manager(db_manager) {}

// === User Management ===

std::optional<int> PlayerRepository::createUser(
    const std::string& username,
    const std::string& email,
    const std::string& password_hash,
    const std::string& salt
) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> int {
            std::string query = R"(
                INSERT INTO users (username, email, password_hash, salt)
                VALUES ($1, $2, $3, $4)
                RETURNING id
            )";

            const char* params[] = {
                username.c_str(),
                email.c_str(),
                password_hash.c_str(),
                salt.c_str()
            };

            auto result = conn.execParams(query, 4, params);
            if (result.isOk() && result.numRows() > 0) {
                return std::stoi(result.getValue(0, 0));
            }
            throw QueryException("CREATE USER", result.getError());
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create user: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<User> PlayerRepository::findUserByUsername(const std::string& username) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> std::optional<User> {
            std::string query = R"(
                SELECT id, username, email, password_hash, salt,
                       email_verified, account_locked, failed_login_attempts,
                       last_failed_login, created_at, updated_at, last_login
                FROM users
                WHERE username = $1
            )";

            const char* params[] = { username.c_str() };
            auto result = conn.execParams(query, 1, params);

            if (result.isOk() && result.numRows() > 0) {
                User user;
                user.id = std::stoi(result.getValue(0, 0));
                user.username = result.getValue(0, 1);
                user.email = result.getValue(0, 2);
                user.password_hash = result.getValue(0, 3);
                user.salt = result.getValue(0, 4);
                user.email_verified = result.getValue(0, 5) == "t";
                user.account_locked = result.getValue(0, 6) == "t";
                user.failed_login_attempts = std::stoi(result.getValue(0, 7));

                if (!result.isNull(0, 8)) {
                    user.last_failed_login = stringToTimestamp(result.getValue(0, 8));
                }

                user.created_at = stringToTimestamp(result.getValue(0, 9));
                user.updated_at = stringToTimestamp(result.getValue(0, 10));

                if (!result.isNull(0, 11)) {
                    user.last_login = stringToTimestamp(result.getValue(0, 11));
                }

                return user;
            }
            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to find user by username: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<User> PlayerRepository::findUserByEmail(const std::string& email) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> std::optional<User> {
            std::string query = R"(
                SELECT id, username, email, password_hash, salt,
                       email_verified, account_locked, failed_login_attempts,
                       last_failed_login, created_at, updated_at, last_login
                FROM users
                WHERE email = $1
            )";

            const char* params[] = { email.c_str() };
            auto result = conn.execParams(query, 1, params);

            if (result.isOk() && result.numRows() > 0) {
                User user;
                user.id = std::stoi(result.getValue(0, 0));
                user.username = result.getValue(0, 1);
                user.email = result.getValue(0, 2);
                user.password_hash = result.getValue(0, 3);
                user.salt = result.getValue(0, 4);
                user.email_verified = result.getValue(0, 5) == "t";
                user.account_locked = result.getValue(0, 6) == "t";
                user.failed_login_attempts = std::stoi(result.getValue(0, 7));

                if (!result.isNull(0, 8)) {
                    user.last_failed_login = stringToTimestamp(result.getValue(0, 8));
                }

                user.created_at = stringToTimestamp(result.getValue(0, 9));
                user.updated_at = stringToTimestamp(result.getValue(0, 10));

                if (!result.isNull(0, 11)) {
                    user.last_login = stringToTimestamp(result.getValue(0, 11));
                }

                return user;
            }
            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to find user by email: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<User> PlayerRepository::findUserById(int user_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> std::optional<User> {
            std::string query = R"(
                SELECT id, username, email, password_hash, salt,
                       email_verified, account_locked, failed_login_attempts,
                       last_failed_login, created_at, updated_at, last_login
                FROM users
                WHERE id = $1
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = { id_str.c_str() };
            auto result = conn.execParams(query, 1, params);

            if (result.isOk() && result.numRows() > 0) {
                User user;
                user.id = std::stoi(result.getValue(0, 0));
                user.username = result.getValue(0, 1);
                user.email = result.getValue(0, 2);
                user.password_hash = result.getValue(0, 3);
                user.salt = result.getValue(0, 4);
                user.email_verified = result.getValue(0, 5) == "t";
                user.account_locked = result.getValue(0, 6) == "t";
                user.failed_login_attempts = std::stoi(result.getValue(0, 7));

                if (!result.isNull(0, 8)) {
                    user.last_failed_login = stringToTimestamp(result.getValue(0, 8));
                }

                user.created_at = stringToTimestamp(result.getValue(0, 9));
                user.updated_at = stringToTimestamp(result.getValue(0, 10));

                if (!result.isNull(0, 11)) {
                    user.last_login = stringToTimestamp(result.getValue(0, 11));
                }

                return user;
            }
            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to find user by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool PlayerRepository::updatePassword(int user_id, const std::string& new_password_hash, const std::string& new_salt) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE users
                SET password_hash = $2, salt = $3, updated_at = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = {
                id_str.c_str(),
                new_password_hash.c_str(),
                new_salt.c_str()
            };

            auto result = conn.execParams(query, 3, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to update password: " + std::string(e.what()));
        return false;
    }
}

bool PlayerRepository::updateLastLogin(int user_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE users
                SET last_login = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = { id_str.c_str() };

            auto result = conn.execParams(query, 1, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to update last login: " + std::string(e.what()));
        return false;
    }
}

int PlayerRepository::incrementFailedLogins(int user_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> int {
            std::string query = R"(
                UPDATE users
                SET failed_login_attempts = failed_login_attempts + 1,
                    last_failed_login = CURRENT_TIMESTAMP
                WHERE id = $1
                RETURNING failed_login_attempts
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = { id_str.c_str() };

            auto result = conn.execParams(query, 1, params);
            if (result.isOk() && result.numRows() > 0) {
                return std::stoi(result.getValue(0, 0));
            }
            return 0;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to increment failed logins: " + std::string(e.what()));
        return 0;
    }
}

bool PlayerRepository::resetFailedLogins(int user_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE users
                SET failed_login_attempts = 0,
                    last_failed_login = NULL
                WHERE id = $1
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = { id_str.c_str() };

            auto result = conn.execParams(query, 1, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to reset failed logins: " + std::string(e.what()));
        return false;
    }
}

bool PlayerRepository::setAccountLocked(int user_id, bool locked) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE users
                SET account_locked = $2,
                    updated_at = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            std::string id_str = std::to_string(user_id);
            std::string locked_str = locked ? "true" : "false";
            const char* params[] = {
                id_str.c_str(),
                locked_str.c_str()
            };

            auto result = conn.execParams(query, 2, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to set account locked status: " + std::string(e.what()));
        return false;
    }
}

bool PlayerRepository::markEmailVerified(int user_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE users
                SET email_verified = true,
                    updated_at = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = { id_str.c_str() };

            auto result = conn.execParams(query, 1, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to mark email as verified: " + std::string(e.what()));
        return false;
    }
}

// === Session Management ===

std::optional<int> PlayerRepository::createSession(const UserSession& session) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> int {
            std::string query = R"(
                INSERT INTO user_sessions (user_id, session_token, refresh_token, expires_at,
                                          refresh_expires_at, ip_address, user_agent, remember_me)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
                RETURNING id
            )";

            std::string user_id_str = std::to_string(session.user_id);
            std::string expires_at_str = timestampToString(session.expires_at);
            std::string refresh_expires_str = session.refresh_expires_at.has_value() ?
                timestampToString(session.refresh_expires_at.value()) : "";
            std::string remember_str = session.remember_me ? "true" : "false";

            const char* params[] = {
                user_id_str.c_str(),
                session.session_token.c_str(),
                session.refresh_token.has_value() ? session.refresh_token->c_str() : nullptr,
                expires_at_str.c_str(),
                refresh_expires_str.empty() ? nullptr : refresh_expires_str.c_str(),
                session.ip_address.has_value() ? session.ip_address->c_str() : nullptr,
                session.user_agent.has_value() ? session.user_agent->c_str() : nullptr,
                remember_str.c_str()
            };

            auto result = conn.execParams(query, 8, params);
            if (result.isOk() && result.numRows() > 0) {
                return std::stoi(result.getValue(0, 0));
            }
            throw QueryException("CREATE SESSION", result.getError());
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create session: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<UserSession> PlayerRepository::findSessionByToken(const std::string& token) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> std::optional<UserSession> {
            std::string query = R"(
                SELECT id, user_id, session_token, refresh_token, expires_at,
                       refresh_expires_at, ip_address, user_agent, remember_me,
                       created_at, last_used, revoked, revoked_at
                FROM user_sessions
                WHERE session_token = $1 AND NOT revoked
                  AND expires_at > CURRENT_TIMESTAMP
            )";

            const char* params[] = { token.c_str() };
            auto result = conn.execParams(query, 1, params);

            if (result.isOk() && result.numRows() > 0) {
                UserSession session;
                session.id = std::stoi(result.getValue(0, 0));
                session.user_id = std::stoi(result.getValue(0, 1));
                session.session_token = result.getValue(0, 2);

                if (!result.isNull(0, 3)) {
                    session.refresh_token = result.getValue(0, 3);
                }

                session.expires_at = stringToTimestamp(result.getValue(0, 4));

                if (!result.isNull(0, 5)) {
                    session.refresh_expires_at = stringToTimestamp(result.getValue(0, 5));
                }

                if (!result.isNull(0, 6)) {
                    session.ip_address = result.getValue(0, 6);
                }

                if (!result.isNull(0, 7)) {
                    session.user_agent = result.getValue(0, 7);
                }

                session.remember_me = result.getValue(0, 8) == "t";
                session.created_at = stringToTimestamp(result.getValue(0, 9));
                session.last_used = stringToTimestamp(result.getValue(0, 10));
                session.revoked = result.getValue(0, 11) == "t";

                if (!result.isNull(0, 12)) {
                    session.revoked_at = stringToTimestamp(result.getValue(0, 12));
                }

                return session;
            }
            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to find session by token: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool PlayerRepository::updateSessionLastUsed(int session_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE user_sessions
                SET last_used = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            std::string id_str = std::to_string(session_id);
            const char* params[] = { id_str.c_str() };

            auto result = conn.execParams(query, 1, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to update session last used: " + std::string(e.what()));
        return false;
    }
}

bool PlayerRepository::revokeSession(int session_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE user_sessions
                SET revoked = true, revoked_at = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            std::string id_str = std::to_string(session_id);
            const char* params[] = { id_str.c_str() };

            auto result = conn.execParams(query, 1, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to revoke session: " + std::string(e.what()));
        return false;
    }
}

int PlayerRepository::revokeAllUserSessions(int user_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> int {
            std::string query = R"(
                UPDATE user_sessions
                SET revoked = true, revoked_at = CURRENT_TIMESTAMP
                WHERE user_id = $1 AND NOT revoked
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = { id_str.c_str() };

            auto result = conn.execParams(query, 1, params);
            if (result.isOk()) {
                // Get affected row count
                std::string count_str = conn.cmdTuples(result.get());
                return count_str.empty() ? 0 : std::stoi(count_str);
            }
            return 0;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to revoke all user sessions: " + std::string(e.what()));
        return 0;
    }
}

int PlayerRepository::cleanupExpiredSessions() {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> int {
            std::string query = R"(
                DELETE FROM user_sessions
                WHERE expires_at < CURRENT_TIMESTAMP
                   OR (revoked = true AND revoked_at < CURRENT_TIMESTAMP - INTERVAL '30 days')
            )";

            auto result = conn.exec(query);
            if (result.isOk()) {
                std::string count_str = conn.cmdTuples(result.get());
                return count_str.empty() ? 0 : std::stoi(count_str);
            }
            return 0;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to cleanup expired sessions: " + std::string(e.what()));
        return 0;
    }
}

// === Profile Management ===

std::optional<UserProfile> PlayerRepository::getUserProfile(int user_id) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> std::optional<UserProfile> {
            std::string query = R"(
                SELECT id, user_id, display_name, avatar_url, timezone,
                       language, theme, privacy_settings, game_settings,
                       created_at, updated_at
                FROM user_profiles
                WHERE user_id = $1
            )";

            std::string id_str = std::to_string(user_id);
            const char* params[] = { id_str.c_str() };
            auto result = conn.execParams(query, 1, params);

            if (result.isOk() && result.numRows() > 0) {
                UserProfile profile;
                profile.id = std::stoi(result.getValue(0, 0));
                profile.user_id = std::stoi(result.getValue(0, 1));

                if (!result.isNull(0, 2)) {
                    profile.display_name = result.getValue(0, 2);
                }
                if (!result.isNull(0, 3)) {
                    profile.avatar_url = result.getValue(0, 3);
                }

                profile.timezone = result.getValue(0, 4);
                profile.language = result.getValue(0, 5);
                profile.theme = result.getValue(0, 6);
                profile.privacy_settings = result.getValue(0, 7);
                profile.game_settings = result.getValue(0, 8);
                profile.created_at = stringToTimestamp(result.getValue(0, 9));
                profile.updated_at = stringToTimestamp(result.getValue(0, 10));

                return profile;
            }
            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get user profile: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool PlayerRepository::upsertUserProfile(const UserProfile& profile) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                INSERT INTO user_profiles (user_id, display_name, avatar_url, timezone,
                                          language, theme, privacy_settings, game_settings)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
                ON CONFLICT (user_id) DO UPDATE SET
                    display_name = EXCLUDED.display_name,
                    avatar_url = EXCLUDED.avatar_url,
                    timezone = EXCLUDED.timezone,
                    language = EXCLUDED.language,
                    theme = EXCLUDED.theme,
                    privacy_settings = EXCLUDED.privacy_settings,
                    game_settings = EXCLUDED.game_settings,
                    updated_at = CURRENT_TIMESTAMP
            )";

            std::string user_id_str = std::to_string(profile.user_id);
            const char* params[] = {
                user_id_str.c_str(),
                profile.display_name.has_value() ? profile.display_name->c_str() : nullptr,
                profile.avatar_url.has_value() ? profile.avatar_url->c_str() : nullptr,
                profile.timezone.c_str(),
                profile.language.c_str(),
                profile.theme.c_str(),
                profile.privacy_settings.c_str(),
                profile.game_settings.c_str()
            };

            auto result = conn.execParams(query, 8, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to upsert user profile: " + std::string(e.what()));
        return false;
    }
}

// === Token Management ===

bool PlayerRepository::createPasswordResetToken(
    int user_id,
    const std::string& token,
    const std::chrono::system_clock::time_point& expires_at
) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                INSERT INTO password_reset_tokens (user_id, token, expires_at)
                VALUES ($1, $2, $3)
            )";

            std::string user_id_str = std::to_string(user_id);
            std::string expires_at_str = timestampToString(expires_at);
            const char* params[] = {
                user_id_str.c_str(),
                token.c_str(),
                expires_at_str.c_str()
            };

            auto result = conn.execParams(query, 3, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create password reset token: " + std::string(e.what()));
        return false;
    }
}

std::optional<int> PlayerRepository::validatePasswordResetToken(const std::string& token) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> std::optional<int> {
            std::string query = R"(
                SELECT user_id
                FROM password_reset_tokens
                WHERE token = $1
                  AND expires_at > CURRENT_TIMESTAMP
                  AND used = false
            )";

            const char* params[] = { token.c_str() };
            auto result = conn.execParams(query, 1, params);

            if (result.isOk() && result.numRows() > 0) {
                return std::stoi(result.getValue(0, 0));
            }
            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to validate password reset token: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool PlayerRepository::markPasswordResetTokenUsed(const std::string& token) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE password_reset_tokens
                SET used = true, used_at = CURRENT_TIMESTAMP
                WHERE token = $1
            )";

            const char* params[] = { token.c_str() };
            auto result = conn.execParams(query, 1, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to mark password reset token as used: " + std::string(e.what()));
        return false;
    }
}

bool PlayerRepository::createEmailVerificationToken(
    int user_id,
    const std::string& token,
    const std::chrono::system_clock::time_point& expires_at
) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                INSERT INTO email_verification_tokens (user_id, token, expires_at)
                VALUES ($1, $2, $3)
            )";

            std::string user_id_str = std::to_string(user_id);
            std::string expires_at_str = timestampToString(expires_at);
            const char* params[] = {
                user_id_str.c_str(),
                token.c_str(),
                expires_at_str.c_str()
            };

            auto result = conn.execParams(query, 3, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create email verification token: " + std::string(e.what()));
        return false;
    }
}

std::optional<int> PlayerRepository::validateEmailVerificationToken(const std::string& token) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> std::optional<int> {
            std::string query = R"(
                SELECT user_id
                FROM email_verification_tokens
                WHERE token = $1
                  AND expires_at > CURRENT_TIMESTAMP
                  AND used = false
            )";

            const char* params[] = { token.c_str() };
            auto result = conn.execParams(query, 1, params);

            if (result.isOk() && result.numRows() > 0) {
                return std::stoi(result.getValue(0, 0));
            }
            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to validate email verification token: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool PlayerRepository::markEmailVerificationTokenUsed(const std::string& token) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                UPDATE email_verification_tokens
                SET used = true, used_at = CURRENT_TIMESTAMP
                WHERE token = $1
            )";

            const char* params[] = { token.c_str() };
            auto result = conn.execParams(query, 1, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to mark email verification token as used: " + std::string(e.what()));
        return false;
    }
}

// === Login History ===

bool PlayerRepository::recordLoginAttempt(
    int user_id,
    bool success,
    const std::string& ip_address,
    const std::string& user_agent,
    const std::string& failure_reason,
    std::optional<int> session_id
) {
    try {
        return db_manager.executeQuery([&](Connection& conn) -> bool {
            std::string query = R"(
                INSERT INTO user_login_history (user_id, success, ip_address, user_agent,
                                               failure_reason, session_id)
                VALUES ($1, $2, $3, $4, $5, $6)
            )";

            std::string user_id_str = std::to_string(user_id);
            std::string success_str = success ? "true" : "false";
            std::string session_id_str = session_id.has_value() ? std::to_string(session_id.value()) : "";

            const char* params[] = {
                user_id_str.c_str(),
                success_str.c_str(),
                ip_address.c_str(),
                user_agent.c_str(),
                failure_reason.empty() ? nullptr : failure_reason.c_str(),
                session_id_str.empty() ? nullptr : session_id_str.c_str()
            };

            auto result = conn.execParams(query, 6, params);
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Failed to record login attempt: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::pair<std::chrono::system_clock::time_point, bool>>
PlayerRepository::getRecentLoginHistory(int user_id, int limit) {
    try {
        return db_manager.executeQuery([&](Connection& conn) ->
            std::vector<std::pair<std::chrono::system_clock::time_point, bool>> {
            std::string query = R"(
                SELECT login_time, success
                FROM user_login_history
                WHERE user_id = $1
                ORDER BY login_time DESC
                LIMIT $2
            )";

            std::string user_id_str = std::to_string(user_id);
            std::string limit_str = std::to_string(limit);
            const char* params[] = {
                user_id_str.c_str(),
                limit_str.c_str()
            };

            auto result = conn.execParams(query, 2, params);
            std::vector<std::pair<std::chrono::system_clock::time_point, bool>> history;

            if (result.isOk()) {
                for (int i = 0; i < result.numRows(); ++i) {
                    auto time = stringToTimestamp(result.getValue(i, 0));
                    bool success = result.getValue(i, 1) == "t";
                    history.emplace_back(time, success);
                }
            }

            return history;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get recent login history: " + std::string(e.what()));
        return {};
    }
}

// === Helper Functions ===

std::string PlayerRepository::timestampToString(const std::chrono::system_clock::time_point& time) const {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::chrono::system_clock::time_point PlayerRepository::stringToTimestamp(const std::string& str) const {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

} // namespace db

#endif // ENABLE_DATABASE