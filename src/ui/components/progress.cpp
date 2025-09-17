#include "ui/components/progress.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <algorithm>
#include <chrono>

using namespace ftxui;

namespace ui::components {

Component Progress::CreateBar(
    float& progress,
    const std::string& label,
    const BarStyle& style) {

    return Renderer([&progress, label, style] {
        // Clamp value between 0 and 1
        float clamped_value = std::max(0.0f, std::min(1.0f, progress));

        // Calculate filled width
        int filled = static_cast<int>(clamped_value * style.width);

        // Build progress bar string
        std::string bar_str;
        for (int i = 0; i < style.width; i++) {
            if (i < filled - 1) {
                bar_str += style.fill_char;
            } else if (i == filled - 1 && filled > 0) {
                bar_str += style.edge_char;
            } else {
                bar_str += style.empty_char;
            }
        }

        Elements elements;

        // Add label if provided
        if (!label.empty()) {
            elements.push_back(text(label));
        }

        auto bar_elem = text(bar_str);

        // Add percentage if requested
        if (style.show_percentage) {
            int percentage = static_cast<int>(clamped_value * 100);
            std::string percent_str = std::to_string(percentage) + "%";

            if (style.show_border) {
                elements.push_back(hbox({
                    text("["),
                    bar_elem | color(style.fill_color),
                    text("] "),
                    text(percent_str)
                }));
            } else {
                elements.push_back(hbox({
                    bar_elem | color(style.fill_color),
                    text(" "),
                    text(percent_str)
                }));
            }
        } else {
            if (style.show_border) {
                elements.push_back(hbox({
                    text("["),
                    bar_elem | color(style.fill_color),
                    text("]")
                }));
            } else {
                elements.push_back(bar_elem | color(style.fill_color));
            }
        }

        return vbox(elements);
    });
}

Component Progress::CreateSpinner(
    bool& is_spinning,
    const std::string& label,
    const SpinnerStyle& style) {

    auto frame_counter = std::make_shared<int>(0);
    auto last_update = std::make_shared<std::chrono::steady_clock::time_point>(
        std::chrono::steady_clock::now());

    return Renderer([&is_spinning, label, style, frame_counter, last_update] {
        if (!is_spinning) {
            return text("");
        }

        // Update frame based on time
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - *last_update).count();

        if (diff >= style.speed) {
            (*frame_counter)++;
            *last_update = now;
        }

        const std::vector<std::string>& frames = style.frames;
        if (frames.empty()) {
            return text("");
        }

        // Use modulo to cycle through frames
        int current_frame = (*frame_counter) % frames.size();

        Elements elements;
        elements.push_back(text(frames[current_frame]) | color(style.color));

        if (!label.empty()) {
            elements.push_back(text(" " + label));
        }

        return hbox(elements);
    });
}

Component Progress::CreateMultiBar(
    const std::vector<std::pair<std::string, float>>& progresses,
    const BarStyle& style) {

    return Renderer([progresses, style] {
        Elements bars;

        for (const auto& [label, value] : progresses) {
            // Clamp value between 0 and 1
            float clamped_value = std::max(0.0f, std::min(1.0f, value));

            // Calculate filled width
            int filled = static_cast<int>(clamped_value * style.width);

            // Build progress bar string
            std::string bar_str;
            for (int i = 0; i < style.width; i++) {
                if (i < filled - 1) {
                    bar_str += style.fill_char;
                } else if (i == filled - 1 && filled > 0) {
                    bar_str += style.edge_char;
                } else {
                    bar_str += style.empty_char;
                }
            }

            auto bar_elem = text(bar_str) | color(style.fill_color);

            Elements row;
            row.push_back(text(label) | size(WIDTH, EQUAL, 15));

            if (style.show_border) {
                row.push_back(text("["));
                row.push_back(bar_elem);
                row.push_back(text("]"));
            } else {
                row.push_back(bar_elem);
            }

            if (style.show_percentage) {
                int percentage = static_cast<int>(clamped_value * 100);
                row.push_back(text(" " + std::to_string(percentage) + "%"));
            }

            bars.push_back(hbox(row));
        }

        return vbox(bars);
    });
}

Element Progress::CreateCircular(
    float progress,
    int size,
    const BarStyle& style) {

    // Clamp value between 0 and 1
    float clamped_value = std::max(0.0f, std::min(1.0f, progress));

    // Simple ASCII circular progress based on size
    std::string circle_str;

    if (size <= 3) {
        // Small circle - simple dot progression
        const std::vector<std::string> small_circles = {
            "○", "◔", "◑", "◕", "●"
        };
        int index = static_cast<int>(clamped_value * (small_circles.size() - 1));
        circle_str = small_circles[index];
    } else {
        // Larger circle - use percentage
        int percentage = static_cast<int>(clamped_value * 100);
        circle_str = "(" + std::to_string(percentage) + "%)";
    }

    auto circle = text(circle_str);

    if (clamped_value >= 1.0f) {
        circle = circle | color(style.fill_color);
    } else if (clamped_value > 0.0f) {
        circle = circle | color(style.empty_color);
    }

    return circle;
}

} // namespace ui::components