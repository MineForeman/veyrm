#pragma once
#include "ui/login_view.h"
#include "auth/login_controller.h"
#include "auth/authentication_service.h"
#include <memory>

namespace ui {

/**
 * @brief Presenter that coordinates between LoginView and LoginController
 * This class implements the MVP pattern, separating UI from business logic
 */
class LoginPresenter {
public:
    /**
     * @brief Constructor
     * @param auth_service Authentication service
     */
    explicit LoginPresenter(auth::AuthenticationService& auth_service);

    /**
     * @brief Run the login flow
     * @return Result of the login interaction
     */
    LoginView::Result run();

    /**
     * @brief Set callback for successful login
     * @param callback Function to call on successful login
     */
    void setOnLoginSuccess(std::function<void(int, const std::string&)> callback);

    /**
     * @brief Set initial mode
     * @param mode Mode to start with
     */
    void setMode(LoginView::Mode mode);

private:
    std::unique_ptr<LoginView> view;
    std::unique_ptr<auth::LoginController> controller;
    std::function<void(int, const std::string&)> on_login_success;

    void setupCallbacks();
};

} // namespace ui