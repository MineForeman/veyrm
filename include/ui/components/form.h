/**
 * @file form.h
 * @brief Reusable form input components
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>
#include <vector>
#include <optional>

namespace ui::components {

/**
 * @class Form
 * @brief Reusable form input components
 */
class Form {
public:
    /**
     * @brief Input field style options
     */
    struct InputStyle {
        ftxui::Color normal_color;
        ftxui::Color focused_color;
        ftxui::Color error_color;
        ftxui::Color placeholder_color;
        bool show_border;
        int width;

        InputStyle() :
            normal_color(ftxui::Color::White),
            focused_color(ftxui::Color::Cyan),
            error_color(ftxui::Color::Red),
            placeholder_color(ftxui::Color::GrayDark),
            show_border(true),
            width(30) {}
    };

    /**
     * @brief Validation result
     */
    struct ValidationResult {
        bool is_valid = true;
        std::string error_message;
    };

    /**
     * @brief Create a text input field
     * @param value Reference to input string
     * @param placeholder Placeholder text
     * @param validator Optional validation function
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateTextInput(
        std::string& value,
        const std::string& placeholder = "",
        std::function<ValidationResult(const std::string&)> validator = nullptr,
        const InputStyle& style = InputStyle{}
    );

    /**
     * @brief Create a password input field
     * @param value Reference to password string
     * @param placeholder Placeholder text
     * @param validator Optional validation function
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreatePasswordInput(
        std::string& value,
        const std::string& placeholder = "Password",
        std::function<ValidationResult(const std::string&)> validator = nullptr,
        const InputStyle& style = InputStyle{}
    );

    /**
     * @brief Create a checkbox
     * @param label Checkbox label
     * @param checked Reference to checked state
     * @param on_change Optional change callback
     * @return FTXUI component
     */
    static ftxui::Component CreateCheckbox(
        const std::string& label,
        bool& checked,
        std::function<void(bool)> on_change = nullptr
    );

    /**
     * @brief Create a radio button group
     * @param options Radio button labels
     * @param selected Reference to selected index
     * @param on_change Optional change callback
     * @return FTXUI component
     */
    static ftxui::Component CreateRadioGroup(
        const std::vector<std::string>& options,
        int& selected,
        std::function<void(int)> on_change = nullptr
    );

    /**
     * @brief Create a dropdown/select component
     * @param options Dropdown options
     * @param selected Reference to selected index
     * @param placeholder Placeholder when nothing selected
     * @param on_change Optional change callback
     * @param style Optional custom style
     * @return FTXUI component
     */
    static ftxui::Component CreateDropdown(
        const std::vector<std::string>& options,
        int& selected,
        const std::string& placeholder = "Select...",
        std::function<void(int)> on_change = nullptr,
        const InputStyle& style = InputStyle{}
    );

    /**
     * @brief Create a complete form with multiple fields
     * @param fields Field components
     * @param on_submit Submit callback
     * @param on_cancel Cancel callback
     * @return FTXUI component
     */
    static ftxui::Component CreateFormContainer(
        const std::vector<std::pair<std::string, ftxui::Component>>& fields,
        std::function<void()> on_submit,
        std::function<void()> on_cancel
    );

    /**
     * @brief Common validators
     */
    class Validators {
    public:
        static ValidationResult Email(const std::string& value);
        static ValidationResult Required(const std::string& value);
        static ValidationResult MinLength(const std::string& value, size_t min);
        static ValidationResult MaxLength(const std::string& value, size_t max);
        static ValidationResult Numeric(const std::string& value);
        static ValidationResult AlphaNumeric(const std::string& value);
    };
};

} // namespace ui::components