#include "ui/components/dialog.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace ui::components {

Component Dialog::CreateMessage(
    const std::string& title,
    const std::string& message,
    std::function<void()> on_close,
    const Style& style) {

    return Renderer([title, message, on_close, style] {
        Elements content;

        // Title
        if (!title.empty()) {
            auto title_elem = text(title) | bold;
            if (style.title_color != Color::Default) {
                title_elem = title_elem | color(style.title_color);
            }
            content.push_back(title_elem | center);
            content.push_back(separator());
        }

        // Message
        auto message_elem = text(message);
        content.push_back(message_elem | center);

        // OK button hint
        content.push_back(text(" "));
        content.push_back(text("[Press Enter to close]") | dim | center);

        auto dialog_box = vbox(content) | border;

        // Apply styling
        if (style.min_width > 0 && style.min_height > 0) {
            dialog_box = dialog_box | size(WIDTH, GREATER_THAN, style.min_width)
                                    | size(HEIGHT, GREATER_THAN, style.min_height);
        }

        return dialog_box | center;
    }) | CatchEvent([on_close](Event event) {
        if (event == Event::Return || event == Event::Escape) {
            if (on_close) {
                on_close();
            }
            return true;
        }
        return false;
    });
}

Component Dialog::CreateConfirmation(
    const std::string& title,
    const std::string& message,
    std::function<void()> on_confirm,
    std::function<void()> on_cancel,
    const Style& style) {

    auto selected = std::make_shared<int>(0); // 0 = Cancel, 1 = Confirm

    return Renderer([title, message, selected, style] {
        Elements content;

        // Title
        if (!title.empty()) {
            auto title_elem = text(title) | bold;
            if (style.title_color != Color::Default) {
                title_elem = title_elem | color(style.title_color);
            }
            content.push_back(title_elem | center);
            content.push_back(separator());
        }

        // Message
        auto message_elem = text(message);
        content.push_back(message_elem | center);
        content.push_back(text(" "));

        // Buttons
        auto cancel_btn = text(" Cancel ");
        auto confirm_btn = text(" OK ");

        if (*selected == 0) {
            cancel_btn = cancel_btn | inverted;
        } else {
            confirm_btn = confirm_btn | inverted;
        }

        content.push_back(hbox({
            cancel_btn,
            text("  "),
            confirm_btn
        }) | center);

        auto dialog_box = vbox(content) | border;

        // Apply styling
        if (style.min_width > 0 && style.min_height > 0) {
            dialog_box = dialog_box | size(WIDTH, GREATER_THAN, style.min_width)
                                    | size(HEIGHT, GREATER_THAN, style.min_height);
        }

        return dialog_box | center;
    }) | CatchEvent([selected, on_confirm, on_cancel](Event event) {
        if (event == Event::ArrowLeft && *selected > 0) {
            (*selected)--;
            return true;
        }
        if (event == Event::ArrowRight && *selected < 1) {
            (*selected)++;
            return true;
        }
        if (event == Event::Return) {
            if (*selected == 0) {
                if (on_cancel) on_cancel();
            } else {
                if (on_confirm) on_confirm();
            }
            return true;
        }
        if (event == Event::Escape) {
            if (on_cancel) on_cancel();
            return true;
        }
        return false;
    });
}

Component Dialog::CreateInput(
    const std::string& title,
    const std::string& prompt,
    std::string& input_value,
    std::function<void(const std::string&)> on_submit,
    std::function<void()> on_cancel,
    const Style& style) {

    auto input_option = InputOption();
    input_option.placeholder = "Enter text...";
    auto input_component = Input(&input_value, input_option);

    return Container::Vertical({
        Renderer([title, prompt, style] {
            Elements content;

            // Title
            if (!title.empty()) {
                auto title_elem = text(title) | bold;
                if (style.title_color != Color::Default) {
                    title_elem = title_elem | color(style.title_color);
                }
                content.push_back(title_elem | center);
                content.push_back(separator());
            }

            // Prompt
            if (!prompt.empty()) {
                content.push_back(text(prompt) | center);
                content.push_back(text(" "));
            }

            auto dialog_box = vbox(content);

            // Apply styling
            if (style.border_color != Color::White) {
                dialog_box = dialog_box | border;
            } else {
                dialog_box = dialog_box | border;
            }

            return dialog_box;
        }),
        input_component,
        Renderer([] {
            return vbox({
                text(" "),
                text("[Enter to submit, ESC to cancel]") | dim | center
            });
        })
    }) | CatchEvent([&input_value, on_submit, on_cancel](Event event) {
        if (event == Event::Return) {
            if (on_submit) {
                on_submit(input_value);
            }
            return true;
        }
        if (event == Event::Escape) {
            if (on_cancel) {
                on_cancel();
            }
            return true;
        }
        return false;
    });
}

Component Dialog::CreateCustom(
    const std::string& title,
    Component content,
    const std::vector<Button>& buttons,
    const Style& style) {

    std::vector<Component> button_components;
    for (const auto& button : buttons) {
        button_components.push_back(
            ftxui::Button(button.label, button.on_click)
        );
    }

    auto button_container = Container::Horizontal(button_components);
    bool has_buttons = !buttons.empty();

    return Renderer([title, content, button_container, has_buttons, style] {
        Elements dialog_elements;

        // Title
        if (!title.empty()) {
            auto title_elem = text(title) | bold;
            if (style.title_color != Color::Default) {
                title_elem = title_elem | color(style.title_color);
            }
            dialog_elements.push_back(title_elem | center);
            dialog_elements.push_back(separator());
        }

        // Content
        dialog_elements.push_back(content->Render());

        // Buttons
        if (has_buttons) {
            dialog_elements.push_back(separator());
            dialog_elements.push_back(button_container->Render() | center);
        }

        auto dialog_content = vbox(dialog_elements) | border;

        // Apply size constraints
        if (style.min_width > 0 && style.min_height > 0) {
            dialog_content = dialog_content | size(WIDTH, GREATER_THAN, style.min_width)
                                            | size(HEIGHT, GREATER_THAN, style.min_height);
        }

        return style.center ? (dialog_content | center) : dialog_content;
    });
}

} // namespace ui::components