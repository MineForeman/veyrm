#pragma once

#include <string>
#include <ftxui/dom/elements.hpp>

/**
 * @enum CloudSyncStatus
 * @brief Status of cloud save synchronization
 */
enum class CloudSyncStatus {
    NOT_AVAILABLE,  // No cloud save support
    OFFLINE,        // Not connected to cloud
    SYNCED,         // Fully synchronized
    UPLOADING,      // Currently uploading
    DOWNLOADING,    // Currently downloading
    PENDING,        // Changes pending upload
    CONFLICT,       // Conflict detected
    ERROR          // Sync error occurred
};

/**
 * @struct CloudSaveInfo
 * @brief Information about cloud save status
 */
struct CloudSaveInfo {
    CloudSyncStatus status = CloudSyncStatus::NOT_AVAILABLE;
    std::string last_sync_time;  // e.g., "2 minutes ago"
    std::string device_name;      // e.g., "Desktop PC"
    std::string conflict_info;    // Details about conflict if any
    int upload_progress = 0;      // 0-100 for progress
    int download_progress = 0;    // 0-100 for progress
};

/**
 * @class CloudSaveIndicator
 * @brief UI helper for displaying cloud save status
 */
class CloudSaveIndicator {
public:
    /**
     * @brief Get status icon for cloud sync status
     * @param status The current sync status
     * @return Unicode string with colored icon
     */
    static std::string getStatusIcon(CloudSyncStatus status);

    /**
     * @brief Get status text description
     * @param status The current sync status
     * @return Human-readable status text
     */
    static std::string getStatusText(CloudSyncStatus status);

    /**
     * @brief Create FTXUI element for cloud status
     * @param info Cloud save information
     * @return FTXUI element for rendering
     */
    static ftxui::Element createStatusElement(const CloudSaveInfo& info);

    /**
     * @brief Create compact icon element
     * @param status The sync status
     * @return Small icon element for inline display
     */
    static ftxui::Element createCompactIcon(CloudSyncStatus status);

    /**
     * @brief Create detailed status panel
     * @param info Cloud save information
     * @return Detailed status panel with all information
     */
    static ftxui::Element createDetailedPanel(const CloudSaveInfo& info);

    /**
     * @brief Create progress bar element
     * @param progress Progress value (0-100)
     * @param label Progress label text
     * @return Progress bar element
     */
    static ftxui::Element createProgressBar(int progress, const std::string& label);

    /**
     * @brief Create conflict resolution dialog content
     * @param local_info Local save information
     * @param cloud_info Cloud save information
     * @return Dialog content for conflict resolution
     */
    static ftxui::Element createConflictDialog(const CloudSaveInfo& local_info,
                                               const CloudSaveInfo& cloud_info);
};