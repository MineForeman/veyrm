#include "login_screen.h"
#include "auth/authentication_service.h"
#include "auth/validation_service.h"
#include "log.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <regex>

using namespace ftxui;

LoginScreen::LoginScreen(auth::AuthenticationService& auth_service)
    : auth_service(auth_service)
    , validator(std::make_unique<auth::ValidationService>()) {
    // Initialize components will be done when run() is called
}

LoginScreen::~LoginScreen() = default; // Destructor defined here where ValidationService is complete

LoginScreen::Result LoginScreen::run() {
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
            exitScreen(Result::CANCELLED);
            return true;
        }
        return false;
    });

    screen.Loop(renderer);

    return result;
}

Component LoginScreen::createLoginForm() {
    // Input fields
    auto username_input_field = Input(&username_input, "Username or Email");

    // Password field - mask input with asterisks
    auto password_input_field = Input(&password_input, "Password");
    auto password_display = Renderer(password_input_field, [&] {
        std::string masked(password_input.size(), '*');
        return hbox({
            text("Password: "),
            text(masked) | border
        });
    });

    auto remember_checkbox = Checkbox("Remember Me", &remember_me);

    // Submit button
    auto submit_button = Button("Login", [&] { handleLogin(); });

    // Cancel button
    auto cancel_button = Button("Cancel", [&] { exitScreen(Result::CANCELLED); });

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

Component LoginScreen::createRegistrationForm() {
    // Input fields
    auto username_input_field = Input(&reg_username_input, "Username");
    auto email_input_field = Input(&reg_email_input, "Email");
    auto password_input_field = Input(&reg_password_input, "Password");
    auto confirm_password_field = Input(&reg_confirm_password_input, "Confirm Password");

    // Submit button
    auto submit_button = Button("Register", [&] { handleRegistration(); });

    // Cancel button
    auto cancel_button = Button("Cancel", [&] { exitScreen(Result::CANCELLED); });

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

Component LoginScreen::createPasswordResetForm() {
    // Input fields
    auto email_input_field = Input(&reset_email_input, "Email Address");
    auto token_input_field = Input(&reset_token_input, "Reset Token (if you have one)");
    auto new_password_field = Input(&reset_new_password_input, "New Password");

    // Submit buttons
    auto request_button = Button("Request Reset", [&] {
        if (!reset_email_input.empty()) {
            auto token = auth_service.requestPasswordReset(reset_email_input);
            if (token.has_value()) {
                // In a real app, this would be sent via email
                reset_token_input = token.value();
                showSuccess("Reset token generated (check email in real app)");
            } else {
                showError("Failed to request password reset");
            }
        } else {
            showError("Please enter your email address");
        }
    });

    auto reset_button = Button("Reset Password", [&] {
        if (!reset_token_input.empty() && !reset_new_password_input.empty()) {
            if (auth_service.resetPassword(reset_token_input, reset_new_password_input)) {
                showSuccess("Password reset successfully! Please login.");
                switchMode(Mode::LOGIN);
            } else {
                showError("Failed to reset password. Invalid or expired token.");
            }
        } else {
            showError("Please enter token and new password");
        }
    });

    auto cancel_button = Button("Cancel", [&] {
        switchMode(Mode::LOGIN);
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

Component LoginScreen::createEmailVerificationForm() {
    // Input field
    auto token_input_field = Input(&verify_token_input, "Verification Token");

    // Submit button
    auto verify_button = Button("Verify Email", [&] { handleEmailVerification(); });

    // Cancel button
    auto cancel_button = Button("Cancel", [&] {
        switchMode(Mode::LOGIN);
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

void LoginScreen::handleLogin() {
    // Clear previous messages
    show_error = false;
    show_success = false;

    // Validate inputs using ValidationService
    if (auto error = validator->validateLoginCredentials(username_input, password_input)) {
        showError(error.value());
        return;
    }

    // Attempt login
    auto login_result = auth_service.login(
        username_input,
        password_input,
        remember_me,
        "127.0.0.1",  // In a real app, get actual IP
        "Veyrm Game Client"
    );

    if (login_result.success) {
        user_id = login_result.user_id.value();
        session_token = login_result.session_token.value();
        refresh_token = login_result.refresh_token.value();

        Log::info("User logged in successfully: ID " + std::to_string(user_id));

        // Call success callback if set
        if (on_login_success) {
            on_login_success(user_id, session_token);
        }

        exitScreen(Result::SUCCESS);
    } else {
        showError(login_result.error_message);
    }
}

void LoginScreen::handleRegistration() {
    // Clear previous messages
    show_error = false;
    show_success = false;

    // Validate inputs using ValidationService
    if (auto error = validator->validateRegistrationData(
        reg_username_input, reg_email_input, reg_password_input, reg_confirm_password_input)) {
        showError(error.value());
        return;
    }

    // Attempt registration
    auto reg_result = auth_service.registerUser(
        reg_username_input,
        reg_email_input,
        reg_password_input
    );

    if (reg_result.success) {
        user_id = reg_result.user_id.value();

        if (reg_result.verification_token.has_value()) {
            // In a real app, this would be sent via email
            verify_token_input = reg_result.verification_token.value();
            showSuccess("Registration successful! Check your email for verification token.");

            // Switch to verification mode
            switchMode(Mode::VERIFY_EMAIL);
        } else {
            showSuccess("Registration successful! Please login.");
            switchMode(Mode::LOGIN);
        }
    } else {
        showError(reg_result.error_message);
    }
}

void LoginScreen::handlePasswordReset() {
    // Handled inline in createPasswordResetForm
}

void LoginScreen::handleEmailVerification() {
    // Clear previous messages
    show_error = false;
    show_success = false;

    if (verify_token_input.empty()) {
        showError("Please enter verification token");
        return;
    }

    if (auth_service.verifyEmail(verify_token_input)) {
        showSuccess("Email verified successfully! Please login.");
        switchMode(Mode::LOGIN);
    } else {
        showError("Invalid or expired verification token");
    }
}

void LoginScreen::switchMode(Mode mode) {
    current_mode = mode;
    clearForms();
    show_error = false;
    show_success = false;
}

void LoginScreen::showError(const std::string& message) {
    error_message = message;
    show_error = true;
    show_success = false;
}

void LoginScreen::showSuccess(const std::string& message) {
    status_message = message;
    show_success = true;
    show_error = false;
}

void LoginScreen::clearForms() {
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

    error_message.clear();
    status_message.clear();
    show_error = false;
    show_success = false;
}


void LoginScreen::exitScreen(Result exit_result) {
    result = exit_result;
    screen.ExitLoopClosure()();
}