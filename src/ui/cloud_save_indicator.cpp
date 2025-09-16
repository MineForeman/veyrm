#include "ui/cloud_save_indicator.h"
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

std::string CloudSaveIndicator::getStatusIcon(CloudSyncStatus status) {
    switch (status) {
        case CloudSyncStatus::NOT_AVAILABLE:
            return "⊗";  // No cloud
        case CloudSyncStatus::OFFLINE:
            return "○";  // Offline
        case CloudSyncStatus::SYNCED:
            return "☁✓"; // Cloud with checkmark
        case CloudSyncStatus::UPLOADING:
            return "☁↑"; // Cloud with up arrow
        case CloudSyncStatus::DOWNLOADING:
            return "☁↓"; // Cloud with down arrow
        case CloudSyncStatus::PENDING:
            return "☁•"; // Cloud with dot
        case CloudSyncStatus::CONFLICT:
            return "☁!"; // Cloud with exclamation
        case CloudSyncStatus::ERROR:
            return "☁✗"; // Cloud with X
        default:
            return "?";
    }
}

std::string CloudSaveIndicator::getStatusText(CloudSyncStatus status) {
    switch (status) {
        case CloudSyncStatus::NOT_AVAILABLE:
            return "Local Only";
        case CloudSyncStatus::OFFLINE:
            return "Offline";
        case CloudSyncStatus::SYNCED:
            return "Synced";
        case CloudSyncStatus::UPLOADING:
            return "Uploading...";
        case CloudSyncStatus::DOWNLOADING:
            return "Downloading...";
        case CloudSyncStatus::PENDING:
            return "Pending Upload";
        case CloudSyncStatus::CONFLICT:
            return "Conflict Detected";
        case CloudSyncStatus::ERROR:
            return "Sync Error";
        default:
            return "Unknown";
    }
}

Element CloudSaveIndicator::createStatusElement(const CloudSaveInfo& info) {
    Color status_color = Color::White;

    switch (info.status) {
        case CloudSyncStatus::SYNCED:
            status_color = Color::Green;
            break;
        case CloudSyncStatus::UPLOADING:
        case CloudSyncStatus::DOWNLOADING:
            status_color = Color::Cyan;
            break;
        case CloudSyncStatus::PENDING:
            status_color = Color::Yellow;
            break;
        case CloudSyncStatus::CONFLICT:
        case CloudSyncStatus::ERROR:
            status_color = Color::Red;
            break;
        case CloudSyncStatus::OFFLINE:
        case CloudSyncStatus::NOT_AVAILABLE:
            status_color = Color::GrayDark;
            break;
    }

    std::string icon = getStatusIcon(info.status);
    std::string status_text = getStatusText(info.status);

    return hbox({
        text(icon) | color(status_color),
        text(" "),
        text(status_text) | color(status_color)
    });
}

Element CloudSaveIndicator::createCompactIcon(CloudSyncStatus status) {
    Color icon_color = Color::White;

    switch (status) {
        case CloudSyncStatus::SYNCED:
            icon_color = Color::Green;
            break;
        case CloudSyncStatus::UPLOADING:
        case CloudSyncStatus::DOWNLOADING:
            icon_color = Color::Cyan;
            break;
        case CloudSyncStatus::PENDING:
            icon_color = Color::Yellow;
            break;
        case CloudSyncStatus::CONFLICT:
        case CloudSyncStatus::ERROR:
            icon_color = Color::Red;
            break;
        default:
            icon_color = Color::GrayDark;
            break;
    }

    return text(getStatusIcon(status)) | color(icon_color);
}

Element CloudSaveIndicator::createDetailedPanel(const CloudSaveInfo& info) {
    std::vector<Element> elements;

    // Status line
    elements.push_back(hbox({
        text("Status: ") | bold,
        createStatusElement(info)
    }));

    // Last sync time if available
    if (!info.last_sync_time.empty() && info.status != CloudSyncStatus::NOT_AVAILABLE) {
        elements.push_back(hbox({
            text("Last Sync: ") | bold,
            text(info.last_sync_time)
        }));
    }

    // Device name if available
    if (!info.device_name.empty()) {
        elements.push_back(hbox({
            text("Device: ") | bold,
            text(info.device_name)
        }));
    }

    // Progress bars for upload/download
    if (info.status == CloudSyncStatus::UPLOADING && info.upload_progress > 0) {
        elements.push_back(createProgressBar(info.upload_progress, "Uploading"));
    } else if (info.status == CloudSyncStatus::DOWNLOADING && info.download_progress > 0) {
        elements.push_back(createProgressBar(info.download_progress, "Downloading"));
    }

    // Conflict info if present
    if (info.status == CloudSyncStatus::CONFLICT && !info.conflict_info.empty()) {
        elements.push_back(separator());
        elements.push_back(text("Conflict:") | bold | color(Color::Red));
        elements.push_back(text(info.conflict_info) | color(Color::Yellow));
    }

    return vbox(elements) | border;
}

Element CloudSaveIndicator::createProgressBar(int progress, const std::string& label) {
    int bar_width = 20;
    int filled = (progress * bar_width) / 100;

    std::string bar = "[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < filled) {
            bar += "=";
        } else if (i == filled) {
            bar += ">";
        } else {
            bar += " ";
        }
    }
    bar += "]";

    return hbox({
        text(label + ": ") | bold,
        text(bar) | color(Color::Cyan),
        text(" " + std::to_string(progress) + "%")
    });
}

Element CloudSaveIndicator::createConflictDialog(const CloudSaveInfo& local_info,
                                                 const CloudSaveInfo& cloud_info) {
    return vbox({
        text("SAVE CONFLICT DETECTED") | bold | color(Color::Red) | center,
        separator(),
        text(""),
        hbox({
            // Local save column
            vbox({
                text("LOCAL SAVE") | bold | center,
                separator(),
                text("Device: " + local_info.device_name),
                text("Modified: " + local_info.last_sync_time),
                text(""),
                text("[1] Keep Local") | color(Color::Yellow)
            }) | border | flex,

            text(" "),

            // Cloud save column
            vbox({
                text("CLOUD SAVE") | bold | center,
                separator(),
                text("Device: " + cloud_info.device_name),
                text("Modified: " + cloud_info.last_sync_time),
                text(""),
                text("[2] Use Cloud") | color(Color::Cyan)
            }) | border | flex
        }),
        text(""),
        text("[3] Keep Both (create backup)") | color(Color::Green) | center,
        text("[ESC] Cancel") | dim | center
    });
}