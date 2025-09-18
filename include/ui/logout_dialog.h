#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>

/**
 * @class LogoutDialog
 * @brief Confirmation dialog for user logout
 */
class LogoutDialog {
public:
    enum class Result {
        NONE,
        LOGOUT,
        CANCEL
    };

    /**
     * @brief Create a logout confirmation dialog component
     * @param on_logout Callback when user confirms logout
     * @param on_cancel Callback when user cancels
     * @return FTXUI component for the dialog
     */
    static ftxui::Component create(
        std::function<void()> on_logout,
        std::function<void()> on_cancel);

    /**
     * @brief Create a logout confirmation dialog element
     * @param username Username to display (optional)
     * @param has_unsaved Whether there are unsaved changes
     * @return FTXUI element for rendering
     */
    static ftxui::Element createDialogElement(
        const std::string& username = "",
        bool has_unsaved = false);

    /**
     * @brief Create a simple yes/no dialog component
     * @param message The message to display
     * @param on_yes Callback for yes
     * @param on_no Callback for no
     * @return FTXUI component
     */
    static ftxui::Component createSimpleDialog(
        const std::string& message,
        std::function<void()> on_yes,
        std::function<void()> on_no);
};