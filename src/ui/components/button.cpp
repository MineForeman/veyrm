#include "ui/components/button.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace ui::components {

Component Button::Create(
    const std::string& label,
    std::function<void()> on_click,
    const Style& style) {
    (void)style; // TODO: Apply style

    auto option = ButtonOption();
    option.on_click = on_click;

    return ftxui::Button(label, on_click, option);
}

Component Button::CreateToggle(
    const std::string& label_on,
    const std::string& label_off,
    bool& is_on,
    std::function<void(bool)> on_toggle,
    const Style& style) {

    return Renderer([&is_on, label_on, label_off, on_toggle, style] {
        std::string label = is_on ? label_on : label_off;
        auto elem = text(label);

        if (style.bold_text) {
            elem = elem | bold;
        }

        elem = elem | color(is_on ? style.pressed_color : style.normal_color);

        if (style.with_border) {
            elem = elem | border;
        }

        return elem;
    }) | CatchEvent([&is_on, on_toggle](Event event) {
        if (event == Event::Return || event == Event::Character(' ')) {
            is_on = !is_on;
            if (on_toggle) {
                on_toggle(is_on);
            }
            return true;
        }
        return false;
    });
}

Component Button::CreateGroup(
    const std::vector<std::string>& labels,
    int& selected,
    std::function<void(int)> on_select,
    const Style& style) {
    (void)style; // TODO: Apply style

    auto option = RadioboxOption();
    // RadioboxOption.on_change is std::function<void()>, not std::function<void(int)>
    // The selected index is managed by the component itself through the reference
    if (on_select) {
        option.on_change = [&selected, on_select]() {
            on_select(selected);
        };
    }

    return Radiobox(&labels, &selected, option);
}

} // namespace ui::components