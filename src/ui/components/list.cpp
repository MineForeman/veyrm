#include "ui/components/list.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>

using namespace ftxui;

namespace ui::components {

Component List::CreateSimple(
    const std::vector<std::string>& items,
    int& selected,
    std::function<void(int)> on_select,
    const Style& style) {
    (void)on_select; // TODO: Hook up callback
    (void)style; // TODO: Apply style

    auto option = MenuOption();
    // MenuOption doesn't directly support on_change with int parameter
    // We'll handle selection through the Menu component itself

    // FTXUI doesn't support const vector for Menu, need to copy
    std::vector<std::string> mutable_items = items;
    return Menu(&mutable_items, &selected, option);
}

Component List::CreateDetailed(
    const std::vector<Item>& items,
    int& selected,
    std::function<void(int)> on_select,
    const Style& style) {

    return Renderer([&items, &selected, on_select, style] {
        Elements elements;

        for (size_t i = 0; i < items.size(); i++) {
            const auto& item = items[i];

            auto label_elem = text(item.label);
            if (!item.enabled) {
                label_elem = label_elem | color(style.disabled_color);
            } else if (static_cast<int>(i) == selected) {
                label_elem = label_elem | color(style.selected_color) | inverted;
            } else {
                label_elem = label_elem | color(style.normal_color);
            }

            Elements row_elements;
            if (!item.icon.empty()) {
                row_elements.push_back(text(item.icon + " "));
            }
            row_elements.push_back(label_elem);

            if (!item.description.empty()) {
                row_elements.push_back(text(" - " + item.description) | color(style.disabled_color));
            }

            elements.push_back(hbox(row_elements));
        }

        auto result = vbox(elements);
        if (style.show_border) {
            result = result | border;
        }

        return result;
    }) | CatchEvent([&items, &selected, on_select](Event event) {
        if (event == Event::ArrowUp && selected > 0) {
            selected--;
            if (on_select) on_select(selected);
            return true;
        }
        if (event == Event::ArrowDown && selected < static_cast<int>(items.size()) - 1) {
            selected++;
            if (on_select) on_select(selected);
            return true;
        }
        if (event == Event::Return && selected >= 0 && selected < static_cast<int>(items.size())) {
            if (items[selected].enabled && on_select) {
                on_select(selected);
            }
            return true;
        }
        return false;
    });
}

Component List::CreateMultiSelect(
    const std::vector<std::string>& items,
    std::vector<bool>& selected,
    std::function<void(int, bool)> on_toggle,
    const Style& style) {
    (void)selected; // Mark as intentionally unused for now

    // Ensure selected vector matches items size
    if (selected.size() != items.size()) {
        selected.resize(items.size(), false);
    }

    return Renderer([&items, &selected, on_toggle, style] {
        Elements elements;

        for (size_t i = 0; i < items.size(); i++) {
            std::string prefix = selected[i] ? "[X] " : "[ ] ";
            auto elem = text(prefix + items[i]);

            if (selected[i]) {
                elem = elem | color(style.highlight_color);
            }

            elements.push_back(elem);
        }

        if (style.show_border) {
            return vbox(elements) | border;
        }
        return vbox(elements);
    });
}

Component List::CreateSearchable(
    const std::vector<std::string>& items,
    std::string& search_query,
    int& selected,
    std::function<void(int)> on_select,
    const Style& style) {
    (void)selected; // TODO: Implement proper selection handling

    // Create filtered items list
    auto filtered_items = std::make_shared<std::vector<std::string>>();
    auto filtered_indices = std::make_shared<std::vector<int>>();

    auto update_filter = [&items, &search_query, filtered_items, filtered_indices]() {
        filtered_items->clear();
        filtered_indices->clear();

        for (size_t i = 0; i < items.size(); i++) {
            if (search_query.empty() ||
                items[i].find(search_query) != std::string::npos) {
                filtered_items->push_back(items[i]);
                filtered_indices->push_back(static_cast<int>(i));
            }
        }
    };

    // Initial filter
    update_filter();

    auto input_option = InputOption();
    input_option.placeholder = "Search...";
    auto search_input = Input(&search_query, input_option);
    auto filtered_selected = std::make_shared<int>(0);

    auto list_component = CreateSimple(
        *filtered_items,
        *filtered_selected,
        [filtered_indices, filtered_selected, on_select](int) {
            if (*filtered_selected >= 0 &&
                *filtered_selected < static_cast<int>(filtered_indices->size())) {
                if (on_select) {
                    on_select((*filtered_indices)[*filtered_selected]);
                }
            }
        },
        style
    );

    return Container::Vertical({
        search_input,
        list_component
    });
}

} // namespace ui::components