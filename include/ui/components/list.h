/**
 * @file list.h
 * @brief Reusable list/menu components
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
 * @class List
 * @brief Reusable list/menu components
 */
class List {
public:
    /**
     * @brief List item definition
     */
    struct Item {
        std::string label;
        std::string description;
        bool enabled = true;
        bool selected = false;
        std::string icon;  // Optional icon/prefix
    };

    /**
     * @brief List style options
     */
    struct Style {
        ftxui::Color normal_color;
        ftxui::Color selected_color;
        ftxui::Color disabled_color;
        ftxui::Color highlight_color;
        bool show_scrollbar;
        bool show_border;
        int max_height;

        Style()
            : normal_color(ftxui::Color::White)
            , selected_color(ftxui::Color::Cyan)
            , disabled_color(ftxui::Color::GrayDark)
            , highlight_color(ftxui::Color::Yellow)
            , show_scrollbar(true)
            , show_border(true)
            , max_height(20) {}
    };

    /**
     * @brief Create a simple list component
     * @param items List items
     * @param selected Reference to selected index
     * @param on_select Callback when selection changes
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateSimple(
        const std::vector<std::string>& items,
        int& selected,
        std::function<void(int)> on_select,
        const Style& style = Style{}
    );

    /**
     * @brief Create a detailed list with descriptions
     * @param items List items with details
     * @param selected Reference to selected index
     * @param on_select Callback when selection changes
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateDetailed(
        const std::vector<Item>& items,
        int& selected,
        std::function<void(int)> on_select,
        const Style& style = Style{}
    );

    /**
     * @brief Create a multi-select list
     * @param items List items
     * @param selected Reference to selected flags vector
     * @param on_toggle Callback when item is toggled
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateMultiSelect(
        const std::vector<std::string>& items,
        std::vector<bool>& selected,
        std::function<void(int, bool)> on_toggle,
        const Style& style = Style{}
    );

    /**
     * @brief Create a searchable list
     * @param items List items
     * @param search_query Reference to search string
     * @param selected Reference to selected index
     * @param on_select Callback when selection changes
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateSearchable(
        const std::vector<std::string>& items,
        std::string& search_query,
        int& selected,
        std::function<void(int)> on_select,
        const Style& style = Style{}
    );
};

} // namespace ui::components