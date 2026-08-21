#include "controller/LoginController.h"

#include "controller/LoginViewContract.h"
#include "model/AuthenticationService.h"

LoginController::LoginController(AuthenticationService &authentifizierung, LoginViewContract &ansicht)
    : m_authentication(authentifizierung)
    , m_view(ansicht)
{
}

void LoginController::loginRequested()
{
    m_view.clearErrorDisplay();

    const AnmeldeErgebnis result = m_authentication.login(m_view.username(), m_view.password());

    if (!result.successful) {
        m_view.showError(result.errorMessage);
        return;
    }
    m_view.loginCompleted(result.user);
}

void LoginController::logoutRequested()
{
    m_authentication.logout();
    m_view.logoutCompleted();
}
