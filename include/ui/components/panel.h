/**
 * @file panel.h
 * @brief Reusable panel/container components
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
 * @class Panel
 * @brief Reusable panel/container components
 */
class Panel {
public:
    /**
     * @brief Panel style options
     */
    struct Style {
        ftxui::Color border_color;
        ftxui::Color title_color;
        ftxui::Color background_color;
        bool show_border;
        bool show_title;
        bool rounded_corners;
        int padding;

        Style() :
            border_color(ftxui::Color::White),
            title_color(ftxui::Color::Cyan),
            background_color(ftxui::Color::Default),
            show_border(true),
            show_title(true),
            rounded_corners(false),
            padding(1) {}
    };

    /**
     * @brief Create a bordered panel
     * @param title Panel title
     * @param content Content component
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateBordered(
        const std::string& title,
        ftxui::Component content,
        const Style& style = Style{}
    );

    /**
     * @brief Create a tabbed panel
     * @param tabs Tab definitions (title, content)
     * @param selected Reference to selected tab index
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateTabbed(
        const std::vector<std::pair<std::string, ftxui::Component>>& tabs,
        int& selected,
        const Style& style = Style{}
    );

    /**
     * @brief Create a split panel (horizontal or vertical)
     * @param left_or_top First panel
     * @param right_or_bottom Second panel
     * @param split_ratio Split ratio (0.0 to 1.0)
     * @param is_horizontal true for horizontal split
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateSplit(
        ftxui::Component left_or_top,
        ftxui::Component right_or_bottom,
        float split_ratio = 0.5f,
        bool is_horizontal = true,
        const Style& style = Style{}
    );

    /**
     * @brief Create a collapsible panel
     * @param title Panel title
     * @param content Content component
     * @param is_expanded Reference to expanded state
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateCollapsible(
        const std::string& title,
        ftxui::Component content,
        bool& is_expanded,
        const Style& style = Style{}
    );

    /**
     * @brief Create a scrollable panel
     * @param content Content component
     * @param max_height Maximum height before scrolling
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateScrollable(
        ftxui::Component content,
        int max_height,
        const Style& style = Style{}
    );
};

} // namespace ui::components