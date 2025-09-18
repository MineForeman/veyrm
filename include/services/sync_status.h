/**
 * @file sync_status.h
 * @brief Common synchronization status enum
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

/**
 * @enum SyncStatus
 * @brief Status of save synchronization
 */
enum class SyncStatus {
    SYNCED,         ///< Local and cloud in sync
    PENDING_UPLOAD, ///< Local changes need upload
    PENDING_DOWNLOAD, ///< Cloud has newer version
    CONFLICT,       ///< Local and cloud diverged
    OFFLINE,        ///< No cloud connection
    ERROR           ///< Sync error occurred
};