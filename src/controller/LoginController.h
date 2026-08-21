// ---------------------------------------------------------------------------
// LoginController — MVC layer: CONTROLLER (US-01)
// View (Signal) -> loginRequested() -> AuthenticationService -> View
// ---------------------------------------------------------------------------
#pragma once

class AuthenticationService;
class LoginViewContract;

class LoginController
{
public:
    LoginController(AuthenticationService &authentifizierung, LoginViewContract &ansicht);

    void loginRequested();
    void logoutRequested();

private:
    AuthenticationService &m_authentication;
    LoginViewContract            &m_view;
};
