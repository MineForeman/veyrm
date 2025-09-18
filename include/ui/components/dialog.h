/**
 * @file dialog.h
 * @brief Reusable dialog/modal component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>
#include <vector>

namespace ui::components {

/**
 * @class Dialog
 * @brief A reusable dialog/modal component
 */
class Dialog {
public:
    /**
     * @brief Dialog button definition
     */
    struct Button {
        std::string label;
        std::function<void()> on_click;
        bool is_default = false;
        bool is_cancel = false;
    };

    /**
     * @brief Dialog style options
     */
    struct Style {
        int min_width;
        int min_height;
        ftxui::Color border_color;
        ftxui::Color title_color;
        bool center;
        bool dim_background;

        Style()
            : min_width(40)
            , min_height(10)
            , border_color(ftxui::Color::White)
            , title_color(ftxui::Color::Cyan)
            , center(true)
            , dim_background(true) {}
    };

    /**
     * @brief Create a simple message dialog
     * @param title Dialog title
     * @param message Dialog message
     * @param on_ok Callback when OK is clicked
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateMessage(
        const std::string& title,
        const std::string& message,
        std::function<void()> on_ok,
        const Style& style = Style{}
    );

    /**
     * @brief Create a confirmation dialog
     * @param title Dialog title
     * @param message Confirmation message
     * @param on_yes Callback when Yes is clicked
     * @param on_no Callback when No is clicked
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateConfirmation(
        const std::string& title,
        const std::string& message,
        std::function<void()> on_yes,
        std::function<void()> on_no,
        const Style& style = Style{}
    );

    /**
     * @brief Create a custom dialog with multiple buttons
     * @param title Dialog title
     * @param content Content component
     * @param buttons Button definitions
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateCustom(
        const std::string& title,
        ftxui::Component content,
        const std::vector<Button>& buttons,
        const Style& style = Style{}
    );

    /**
     * @brief Create an input dialog
     * @param title Dialog title
     * @param prompt Input prompt
     * @param input_value Reference to input string
     * @param on_ok Callback when OK is clicked
     * @param on_cancel Callback when cancelled
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateInput(
        const std::string& title,
        const std::string& prompt,
        std::string& input_value,
        std::function<void(const std::string&)> on_ok,
        std::function<void()> on_cancel,
        const Style& style = Style{}
    );
};

} // namespace ui::components