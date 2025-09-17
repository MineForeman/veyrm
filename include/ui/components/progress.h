/**
 * @file progress.h
 * @brief Reusable progress bar and spinner components
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

namespace ui::components {

/**
 * @class Progress
 * @brief Reusable progress indicators
 */
class Progress {
public:
    /**
     * @brief Progress bar style options
     */
    struct BarStyle {
        int width;
        ftxui::Color fill_color;
        ftxui::Color empty_color;
        char fill_char;
        char empty_char;
        char edge_char;
        bool show_percentage;
        bool show_border;

        BarStyle() :
            width(40),
            fill_color(ftxui::Color::Green),
            empty_color(ftxui::Color::GrayDark),
            fill_char('='),
            empty_char('-'),
            edge_char('>'),
            show_percentage(true),
            show_border(true) {}
    };

    /**
     * @brief Spinner style options
     */
    struct SpinnerStyle {
        std::vector<std::string> frames;
        ftxui::Color color;
        int speed; // milliseconds per frame

        SpinnerStyle() :
            frames({"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"}),
            color(ftxui::Color::Cyan),
            speed(100) {}
    };

    /**
     * @brief Create a progress bar component
     * @param progress Progress value (0.0 to 1.0)
     * @param label Optional label
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateBar(
        float& progress,
        const std::string& label = "",
        const BarStyle& style = BarStyle{}
    );

    /**
     * @brief Create a spinner/loading indicator
     * @param is_spinning Reference to spinning state
     * @param label Optional label
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateSpinner(
        bool& is_spinning,
        const std::string& label = "",
        const SpinnerStyle& style = SpinnerStyle{}
    );

    /**
     * @brief Create a multi-progress bar (stacked)
     * @param progresses Progress values with labels
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateMultiBar(
        const std::vector<std::pair<std::string, float>>& progresses,
        const BarStyle& style = BarStyle{}
    );

    /**
     * @brief Create a circular progress indicator
     * @param progress Progress value (0.0 to 1.0)
     * @param size Circle size
     * @param style Optional custom style
     * @return FTXUI element (not interactive)
     */
    static ftxui::Element CreateCircular(
        float progress,
        int size = 5,
        const BarStyle& style = BarStyle{}
    );
};

} // namespace ui::components