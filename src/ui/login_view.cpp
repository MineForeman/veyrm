#include "ui/login_view.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

namespace ui {

LoginView::LoginView() {
    // Constructor - UI setup will be done in run()
}

void LoginView::setControllerCallbacks(const ControllerCallbacks& callbacks) {
    controller_callbacks = callbacks;
}

LoginView::Result LoginView::run() {
    // Reset state
    result = Result::CANCELLED;
    clearForms();

    // Create the main container based on current mode
    auto main_container = Container::Vertical({});

    // Tab selector for mode
    int tab_selected = static_cast<int>(current_mode);
    std::vector<std::string> tab_entries = {"Login", "Register", "Forgot Password", "Verify Email"};

    auto tab_toggle = Toggle(&tab_entries, &tab_selected);

    // Mode change handler
    auto previous_tab = tab_selected;

    // Content container that changes based on mode
    Component content;

    auto update_content = [&]() {
        main_container->DetachAllChildren();

        switch(static_cast<Mode>(tab_selected)) {
            case Mode::LOGIN:
                content = createLoginForm();
                break;
            case Mode::REGISTER:
                content = createRegistrationForm();
                break;
            case Mode::FORGOT_PASSWORD:
                content = createPasswordResetForm();
                break;
            case Mode::VERIFY_EMAIL:
                content = createEmailVerificationForm();
                break;
        }

        main_container->Add(content);
    };

    update_content();

    // Main layout
    container = Container::Vertical({
        tab_toggle,
        main_container
    });

    // Renderer
    auto renderer = Renderer(container, [&] {
        // Check if tab changed
        if (tab_selected != previous_tab) {
            previous_tab = tab_selected;
            current_mode = static_cast<Mode>(tab_selected);
            clearForms();
            update_content();
        }

        Elements elements;

        // Title
        elements.push_back(
            hbox({
                text(" "),
                text("VEYRM AUTHENTICATION") | bold | color(Color::Cyan),
                text(" ")
            }) | center
        );

        elements.push_back(separator());

        // Tab selector
        elements.push_back(tab_toggle->Render() | center);
        elements.push_back(separator());

        // Status messages
        if (show_error && !error_message.empty()) {
            elements.push_back(
                text(error_message) | color(Color::Red) | center
            );
        }

        if (show_success && !status_message.empty()) {
            elements.push_back(
                text(status_message) | color(Color::Green) | center
            );
        }

        // Content
        elements.push_back(content->Render());

        // Instructions
        elements.push_back(separator());
        elements.push_back(
            hbox({
                text("Tab: Switch Mode | Enter: Submit | ESC: Cancel") | dim
            }) | center
        );

        return vbox(elements) | border;
    });

    // Event handler
    renderer = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Escape) {
            exitWithResult(Result::CANCELLED);
            return true;
        }
        return false;
    });

    screen.Loop(renderer);

    return result;
}

void LoginView::showError(const std::string& message) {
    error_message = message;
    show_error = true;
    show_success = false;
}

void LoginView::showSuccess(const std::string& message) {
    status_message = message;
    show_success = true;
    show_error = false;
}

void LoginView::clearMessages() {
    show_error = false;
    show_success = false;
    error_message.clear();
    status_message.clear();
}

void LoginView::switchToLogin() {
    current_mode = Mode::LOGIN;
    clearForms();
    clearMessages();
}

void LoginView::switchToVerification() {
    current_mode = Mode::VERIFY_EMAIL;
    clearForms();
    clearMessages();
}

void LoginView::exitWithResult(Result exit_result) {
    result = exit_result;
    screen.ExitLoopClosure()();
}

Component LoginView::createLoginForm() {
    // Input fields
    auto username_input_field = Input(&username_input, "Username or Email");
    auto password_input_field = Input(&password_input, "Password");
    auto remember_checkbox = Checkbox("Remember Me", &remember_me);

    // Submit button
    auto submit_button = Button("Login", [&] { submitLogin(); });

    // Cancel button
    auto cancel_button = Button("Cancel", [&] { exitWithResult(Result::CANCELLED); });

    // Layout
    auto form = Container::Vertical({
        username_input_field,
        password_input_field,
        remember_checkbox,
        Container::Horizontal({
            submit_button,
            cancel_button
        })
    });

    // Renderer
    return Renderer(form, [=, this] {
        std::string masked(password_input.size(), '*');
        return vbox({
            hbox({text("Username: "), username_input_field->Render()}),
            hbox({text("Password: "), text(masked) | border}),
            remember_checkbox->Render(),
            separator(),
            hbox({
                submit_button->Render(),
                text(" "),
                cancel_button->Render()
            }) | center
        }) | border | size(WIDTH, GREATER_THAN, 40) | center;
    });
}

Component LoginView::createRegistrationForm() {
    // Input fields
    auto username_input_field = Input(&reg_username_input, "Username");
    auto email_input_field = Input(&reg_email_input, "Email");
    auto password_input_field = Input(&reg_password_input, "Password");
    auto confirm_password_field = Input(&reg_confirm_password_input, "Confirm Password");

    // Submit button
    auto submit_button = Button("Register", [&] { submitRegistration(); });

    // Cancel button
    auto cancel_button = Button("Cancel", [&] { exitWithResult(Result::CANCELLED); });

    // Layout
    auto form = Container::Vertical({
        username_input_field,
        email_input_field,
        password_input_field,
        confirm_password_field,
        Container::Horizontal({
            submit_button,
            cancel_button
        })
    });

    // Renderer
    return Renderer(form, [=, this] {
        std::string masked_pass(reg_password_input.size(), '*');
        std::string masked_confirm(reg_confirm_password_input.size(), '*');
        return vbox({
            hbox({text("Username: "), username_input_field->Render()}),
            hbox({text("Email: "), email_input_field->Render()}),
            hbox({text("Password: "), text(masked_pass) | border}),
            hbox({text("Confirm: "), text(masked_confirm) | border}),
            separator(),
            hbox({
                submit_button->Render(),
                text(" "),
                cancel_button->Render()
            }) | center
        }) | border | size(WIDTH, GREATER_THAN, 40) | center;
    });
}

Component LoginView::createPasswordResetForm() {
    // Input fields
    auto email_input_field = Input(&reset_email_input, "Email Address");
    auto token_input_field = Input(&reset_token_input, "Reset Token (if you have one)");
    auto new_password_field = Input(&reset_new_password_input, "New Password");

    // Submit buttons
    auto request_button = Button("Request Reset", [&] {
        if (controller_callbacks.onPasswordResetRequest) {
            controller_callbacks.onPasswordResetRequest(reset_email_input);
        }
    });

    auto reset_button = Button("Reset Password", [&] {
        if (controller_callbacks.onPasswordReset) {
            controller_callbacks.onPasswordReset(reset_token_input, reset_new_password_input);
        }
    });

    auto cancel_button = Button("Cancel", [&] {
        switchToLogin();
    });

    // Layout
    auto form = Container::Vertical({
        email_input_field,
        request_button,
        token_input_field,
        new_password_field,
        Container::Horizontal({
            reset_button,
            cancel_button
        })
    });

    // Renderer
    return Renderer(form, [=, this] {
        std::string masked_new(reset_new_password_input.size(), '*');
        return vbox({
            text("Password Reset") | bold | center,
            separator(),
            hbox({text("Email: "), email_input_field->Render()}),
            request_button->Render() | center,
            separator(),
            hbox({text("Token: "), token_input_field->Render()}),
            hbox({text("New Password: "), text(masked_new) | border}),
            separator(),
            hbox({
                reset_button->Render(),
                text(" "),
                cancel_button->Render()
            }) | center
        }) | border | size(WIDTH, GREATER_THAN, 50) | center;
    });
}

Component LoginView::createEmailVerificationForm() {
    // Input field
    auto token_input_field = Input(&verify_token_input, "Verification Token");

    // Submit button
    auto verify_button = Button("Verify Email", [&] {
        if (controller_callbacks.onEmailVerification) {
            controller_callbacks.onEmailVerification(verify_token_input);
        }
    });

    // Cancel button
    auto cancel_button = Button("Cancel", [&] {
        switchToLogin();
    });

    // Layout
    auto form = Container::Vertical({
        token_input_field,
        Container::Horizontal({
            verify_button,
            cancel_button
        })
    });

    // Renderer
    return Renderer(form, [=] {
        return vbox({
            text("Email Verification") | bold | center,
            separator(),
            text("Enter the verification token sent to your email"),
            separator(),
            hbox({text("Token: "), token_input_field->Render()}),
            separator(),
            hbox({
                verify_button->Render(),
                text(" "),
                cancel_button->Render()
            }) | center
        }) | border | size(WIDTH, GREATER_THAN, 50) | center;
    });
}

void LoginView::clearForms() {
    username_input.clear();
    password_input.clear();
    remember_me = false;

    reg_username_input.clear();
    reg_email_input.clear();
    reg_password_input.clear();
    reg_confirm_password_input.clear();

    reset_email_input.clear();
    reset_token_input.clear();
    reset_new_password_input.clear();

    verify_token_input.clear();

    clearMessages();
}

void LoginView::submitLogin() {
    if (controller_callbacks.onLogin) {
        auth::LoginCredentials credentials;
        credentials.username = username_input;
        credentials.password = password_input;
        credentials.remember_me = remember_me;
        controller_callbacks.onLogin(credentials);
    }
}

void LoginView::submitRegistration() {
    if (controller_callbacks.onRegister) {
        auth::RegistrationData data;
        data.username = reg_username_input;
        data.email = reg_email_input;
        data.password = reg_password_input;
        data.confirm_password = reg_confirm_password_input;
        controller_callbacks.onRegister(data);
    }
}

} // namespace ui