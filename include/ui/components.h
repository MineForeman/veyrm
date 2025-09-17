
/**
 * @file components.h
 * @brief Main header for UI component library
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

// Include all UI components
#include "ui/components/button.h"
#include "ui/components/dialog.h"
#include "ui/components/list.h"
#include "ui/components/progress.h"
#include "ui/components/panel.h"
#include "ui/components/form.h"

/**
 * @namespace ui::components
 * @brief Reusable UI components for the game
 *
 * This namespace contains a collection of reusable UI components
 * that follow consistent styling and behavior patterns. These
 * components are built on top of FTXUI and provide higher-level
 * abstractions for common UI patterns.
 *
 * ## Component Categories:
 * - **Buttons**: Simple, toggle, and group buttons
 * - **Dialogs**: Message, confirmation, input, and custom dialogs
 * - **Lists**: Simple, detailed, multi-select, and searchable lists
 * - **Progress**: Progress bars, spinners, and circular indicators
 * - **Panels**: Bordered, tabbed, and split panels
 * - **Forms**: Input fields, checkboxes, and form validation
 *
 * ## Usage Example:
 * ```cpp
 * using namespace ui::components;
 *
 * // Create a simple button
 * auto button = Button::Create("Click me!", []() {
 *     std::cout << "Button clicked!" << std::endl;
 * });
 *
 * // Create a confirmation dialog
 * auto dialog = Dialog::CreateConfirmation(
 *     "Delete Save",
 *     "Are you sure you want to delete this save?",
 *     []() { /* Delete save */ },
 *     []() { /* Cancel */ }
 * );
 * ```
 *
 * ## Styling:
 * All components support custom styling through their respective
 * Style structures. This allows for consistent theming across
 * the application.
 *
 * ## MVC Integration:
 * These components are designed to work well with the MVC pattern:
 * - Components are pure UI elements (View layer)
 * - They communicate through callbacks (to Controller)
 * - They don't contain business logic (handled by Controller/Model)
 */
namespace ui::components {

/**
 * @class Theme
 * @brief Global theme settings for all components
 */
class Theme {
public:
    /**
     * @brief Color scheme
     */
    struct Colors {
        ftxui::Color primary = ftxui::Color::Cyan;
        ftxui::Color secondary = ftxui::Color::Blue;
        ftxui::Color success = ftxui::Color::Green;
        ftxui::Color warning = ftxui::Color::Yellow;
        ftxui::Color danger = ftxui::Color::Red;
        ftxui::Color info = ftxui::Color::White;
        ftxui::Color background = ftxui::Color::Black;
        ftxui::Color text = ftxui::Color::White;
        ftxui::Color text_muted = ftxui::Color::GrayDark;
        ftxui::Color border = ftxui::Color::White;
    };

    /**
     * @brief Get the current theme colors
     * @return Current color scheme
     */
    static const Colors& GetColors();

    /**
     * @brief Set custom theme colors
     * @param colors New color scheme
     */
    static void SetColors(const Colors& colors);

    /**
     * @brief Apply dark theme preset
     */
    static void ApplyDarkTheme();

    /**
     * @brief Apply light theme preset
     */
    static void ApplyLightTheme();

    /**
     * @brief Apply classic terminal theme
     */
    static void ApplyClassicTheme();

private:
    static Colors current_colors;
};

} // namespace ui::components