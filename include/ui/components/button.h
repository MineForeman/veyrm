/**
 * @file button.h
 * @brief Reusable button component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>

namespace ui::components {

/**
 * @class Button
 * @brief A reusable button component with customizable styling
 */
class Button {
public:
    /**
     * @brief Button style options
     */
    struct Style {
        ftxui::Color normal_color;
        ftxui::Color hover_color;
        ftxui::Color pressed_color;
        bool with_border;
        bool bold_text;

        Style()
            : normal_color(ftxui::Color::White)
            , hover_color(ftxui::Color::Cyan)
            , pressed_color(ftxui::Color::Green)
            , with_border(true)
            , bold_text(false) {}
    };

    /**
     * @brief Create a button component
     * @param label Button label text
     * @param on_click Callback when button is clicked
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component Create(
        const std::string& label,
        std::function<void()> on_click,
        const Style& style = Style{}
    );

    /**
     * @brief Create a toggle button that switches between two states
     * @param label_on Label when toggled on
     * @param label_off Label when toggled off
     * @param is_on Reference to toggle state
     * @param on_toggle Callback when toggled
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateToggle(
        const std::string& label_on,
        const std::string& label_off,
        bool& is_on,
        std::function<void(bool)> on_toggle,
        const Style& style = Style{}
    );

    /**
     * @brief Create a button group (radio buttons)
     * @param labels Button labels
     * @param selected Reference to selected index
     * @param on_select Callback when selection changes
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateGroup(
        const std::vector<std::string>& labels,
        int& selected,
        std::function<void(int)> on_select,
        const Style& style = Style{}
    );
};

} // namespace ui::components