#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>

namespace auth {

/**
 * @class SessionManager
 * @brief Manages user session lifecycle including timeouts and refresh
 *
 * Features:
 * - Automatic session timeout detection
 * - Token refresh before expiry
 * - Warning notifications before timeout
 * - Background refresh thread
 */
class SessionManager {
public:
    /**
     * @brief Session status
     */
    enum class Status {
        ACTIVE,           // Session is active and valid
        EXPIRING_SOON,    // Session will expire soon (< 5 minutes)
        EXPIRED,          // Session has expired
        REFRESHING,       // Currently refreshing token
        OFFLINE          // No network connection
    };

    /**
     * @brief Constructor
     * @param refresh_callback Function to call for token refresh
     * @param logout_callback Function to call on session expiry
     */
    SessionManager(std::function<bool(std::string&)> refresh_callback = nullptr,
                   std::function<void()> logout_callback = nullptr);

    ~SessionManager();

    /**
     * @brief Start session with initial token
     * @param token Session token
     * @param expiry_seconds Token lifetime in seconds
     * @param refresh_token Optional refresh token
     */
    void startSession(const std::string& token,
                     int expiry_seconds = 3600,
                     const std::string& refresh_token = "");

    /**
     * @brief End the current session
     */
    void endSession();

    /**
     * @brief Get current session status
     * @return Current status
     */
    Status getStatus() const { return status; }

    /**
     * @brief Check if session is active
     * @return true if session is valid
     */
    bool isActive() const;

    /**
     * @brief Get time until expiry
     * @return Seconds until expiry, 0 if expired
     */
    int getSecondsUntilExpiry() const;

    /**
     * @brief Get formatted time until expiry
     * @return Human-readable time string
     */
    std::string getTimeUntilExpiry() const;

    /**
     * @brief Manually trigger token refresh
     * @return true if refresh succeeded
     */
    bool refreshToken();

    /**
     * @brief Set warning time before expiry
     * @param seconds Seconds before expiry to trigger warning
     */
    void setWarningTime(int seconds) { warning_time_seconds = seconds; }

    /**
     * @brief Set auto-refresh enabled
     * @param enabled If true, auto-refresh before expiry
     */
    void setAutoRefresh(bool enabled) { auto_refresh = enabled; }

    /**
     * @brief Get current session token
     * @return Session token or empty if no session
     */
    std::string getSessionToken() const { return session_token; }

    /**
     * @brief Get refresh token
     * @return Refresh token or empty if none
     */
    std::string getRefreshToken() const { return refresh_token; }

    /**
     * @brief Set callback for warning notifications
     * @param callback Function to call when warning time reached
     */
    void setWarningCallback(std::function<void(int)> callback) {
        warning_callback = callback;
    }

    /**
     * @brief Check if warning should be shown
     * @return true if in warning period
     */
    bool shouldShowWarning() const;

    /**
     * @brief Get warning message
     * @return Warning message for UI display
     */
    std::string getWarningMessage() const;

private:
    // Session data
    std::string session_token;
    std::string refresh_token;
    std::chrono::steady_clock::time_point session_start;
    std::chrono::steady_clock::time_point session_expiry;
    Status status = Status::OFFLINE;

    // Configuration
    int warning_time_seconds = 300;  // 5 minutes before expiry
    bool auto_refresh = true;
    int refresh_before_seconds = 60;  // Refresh 1 minute before expiry

    // Callbacks
    std::function<bool(std::string&)> refresh_callback;
    std::function<void()> logout_callback;
    std::function<void(int)> warning_callback;

    // Background thread
    std::unique_ptr<std::thread> monitor_thread;
    std::atomic<bool> monitoring{false};
    std::atomic<bool> stop_monitoring{false};

    // Helper methods
    void startMonitoring();
    void stopMonitoring();
    void monitorSession();
    void updateStatus();
};