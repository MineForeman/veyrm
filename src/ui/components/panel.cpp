#include "ui/components/panel.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>

using namespace ftxui;

namespace ui::components {

Component Panel::CreateBordered(
    const std::string& title,
    Component content,
    const Style& style) {

    return Renderer([content, title, style] {
        auto panel_content = content->Render();

        // Apply border
        if (style.show_border) {
            panel_content = panel_content | border;
        }

        // Add title if provided
        if (!title.empty() && style.show_title) {
            auto title_elem = text(" " + title + " ") | bold;
            if (style.title_color != Color::Default) {
                title_elem = title_elem | color(style.title_color);
            }

            // Create a bordered panel with title
            return vbox({
                title_elem,
                separator(),
                panel_content
            }) | border;
        }

        // Apply background color if set
        if (style.background_color != Color::Default) {
            panel_content = panel_content | bgcolor(style.background_color);
        }

        return panel_content;
    });
}

Component Panel::CreateTabbed(
    const std::vector<std::pair<std::string, Component>>& tabs,
    int& selected_tab,
    const Style& style) {

    // Ensure we have tabs
    if (tabs.empty()) {
        return Renderer([] { return text("No tabs available"); });
    }

    // Extract tab names and contents
    std::vector<std::string> tab_names;
    std::vector<Component> tab_contents;

    for (const auto& [name, content] : tabs) {
        tab_names.push_back(name);
        tab_contents.push_back(content);
    }

    // Create tab container
    auto tab_container = Container::Tab(
        tab_contents,
        &selected_tab
    );

    return Container::Vertical({
        // Tab headers
        Renderer([tab_names, &selected_tab, style] {
            Elements tab_elements;
            for (size_t i = 0; i < tab_names.size(); i++) {
                auto tab_text = text(" " + tab_names[i] + " ");

                if (static_cast<int>(i) == selected_tab) {
                    tab_text = tab_text | inverted;
                    if (style.title_color != Color::Default) {
                        tab_text = tab_text | color(style.title_color);
                    }
                }

                tab_elements.push_back(tab_text);
                if (i < tab_names.size() - 1) {
                    tab_elements.push_back(text(" | "));
                }
            }
            return hbox(tab_elements);
        }),
        Renderer([] { return separator(); }),
        tab_container
    });
}

Component Panel::CreateSplit(
    Component left_or_top,
    Component right_or_bottom,
    float split_ratio,
    bool is_horizontal,
    const Style& style) {

    (void)split_ratio; // Not used for now

    if (is_horizontal) {
        return Container::Horizontal({
            left_or_top | flex,
            Renderer([style] {
                if (style.show_border) {
                    return separatorCharacter("│");
                }
                return text("");
            }),
            right_or_bottom | flex
        });
    } else {
        return Container::Vertical({
            left_or_top | flex,
            Renderer([style] {
                if (style.show_border) {
                    return separator();
                }
                return text("");
            }),
            right_or_bottom | flex
        });
    }
}

Component Panel::CreateCollapsible(
    const std::string& title,
    Component content,
    bool& is_expanded,
    const Style& style) {

    return Renderer([title, content, &is_expanded, style] {
        Elements elements;

        // Create header with expand/collapse indicator
        std::string indicator = is_expanded ? "▼ " : "▶ ";
        auto header = hbox({
            text(indicator),
            text(title) | bold
        });

        if (style.title_color != Color::Default) {
            header = header | color(style.title_color);
        }

        elements.push_back(header);

        // Add content if expanded
        if (is_expanded) {
            elements.push_back(separator());
            elements.push_back(content->Render());
        }

        auto panel = vbox(elements);

        // Apply border if requested
        if (style.show_border) {
            panel = panel | border;
        }

        return panel;
    }) | CatchEvent([&is_expanded](Event event) {
        if (event == Event::Return || event == Event::Character(' ')) {
            is_expanded = !is_expanded;
            return true;
        }
        return false;
    });
}

Component Panel::CreateScrollable(
    Component content,
    int max_height,
    const Style& style) {
    (void)style; // Not used for now

    // FTXUI has built-in scrollable support through Scroller
    auto scrollable_content = content | vscroll_indicator | frame;

    if (max_height > 0) {
        scrollable_content = scrollable_content | size(HEIGHT, LESS_THAN, max_height);
    }

    return scrollable_content;
}

} // namespace ui::components