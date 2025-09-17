#include "ui/components/form.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>
#include <regex>

using namespace ftxui;

namespace ui::components {

Component Form::CreateTextInput(
    std::string& value,
    const std::string& placeholder,
    std::function<ValidationResult(const std::string&)> validator,
    const InputStyle& style) {

    auto input_option = InputOption();
    input_option.placeholder = placeholder;

    auto input = Input(&value, input_option);

    return Renderer(input, [&value, input, validator, style] {
        auto elem = input->Render();

        if (style.show_border) {
            elem = elem | border;
        }

        // Validate and show error if needed
        if (validator) {
            auto result = validator(value);
            if (!result.is_valid && !result.error_message.empty()) {
                return vbox({
                    elem,
                    text(result.error_message) | color(style.error_color)
                });
            }
        }

        return elem;
    });
}

Component Form::CreatePasswordInput(
    std::string& value,
    const std::string& placeholder,
    std::function<ValidationResult(const std::string&)> validator,
    const InputStyle& style) {

    auto input_option = InputOption();
    input_option.placeholder = placeholder;
    input_option.password = true;

    auto input = Input(&value, input_option);

    return Renderer(input, [&value, input, validator, style] {
        auto elem = input->Render();

        if (style.show_border) {
            elem = elem | border;
        }

        // Validate and show error if needed
        if (validator) {
            auto result = validator(value);
            if (!result.is_valid && !result.error_message.empty()) {
                return vbox({
                    elem,
                    text(result.error_message) | color(style.error_color)
                });
            }
        }

        return elem;
    });
}

Component Form::CreateCheckbox(
    const std::string& label,
    bool& checked,
    std::function<void(bool)> on_change) {

    auto checkbox_option = CheckboxOption();
    if (on_change) {
        checkbox_option.on_change = [&checked, on_change]() {
            on_change(checked);
        };
    }

    return Checkbox(label, &checked, checkbox_option);
}

Component Form::CreateRadioGroup(
    const std::vector<std::string>& options,
    int& selected,
    std::function<void(int)> on_change) {

    auto radiobox_option = RadioboxOption();
    if (on_change) {
        radiobox_option.on_change = [&selected, on_change]() {
            on_change(selected);
        };
    }

    return Radiobox(&options, &selected, radiobox_option);
}

Component Form::CreateDropdown(
    const std::vector<std::string>& options,
    int& selected,
    const std::string& placeholder,
    std::function<void(int)> on_change,
    const InputStyle& style) {

    (void)placeholder; // TODO: Use for empty state

    auto menu_option = MenuOption();
    if (on_change) {
        menu_option.on_change = [&selected, on_change]() {
            on_change(selected);
        };
    }
    auto menu = Menu(&options, &selected, menu_option);

    return Renderer(menu, [menu, style, &options, &selected] {
        auto elem = menu->Render();

        if (style.show_border) {
            elem = elem | border;
        }

        // Show selected item
        if (selected >= 0 && selected < static_cast<int>(options.size())) {
            return vbox({
                text("Selected: " + options[selected]) | dim,
                elem
            });
        }

        return elem;
    }) | size(HEIGHT, LESS_THAN, 5);
}


Component Form::CreateFormContainer(
    const std::vector<std::pair<std::string, Component>>& fields,
    std::function<void()> on_submit,
    std::function<void()> on_cancel) {

    std::vector<Component> form_components;

    // Add field components with labels
    for (const auto& [label, component] : fields) {
        if (!label.empty()) {
            form_components.push_back(
                Container::Horizontal({
                    Renderer([label] {
                        return text(label + ": ") | size(WIDTH, EQUAL, 15);
                    }),
                    component
                })
            );
        } else {
            form_components.push_back(component);
        }
    }

    // Add buttons
    auto submit_button = Button("Submit", on_submit);
    auto cancel_button = Button("Cancel", on_cancel);

    form_components.push_back(
        Container::Horizontal({
            submit_button,
            Renderer([] { return text("  "); }),
            cancel_button
        })
    );

    return Container::Vertical(form_components) | border;
}


} // namespace ui::components