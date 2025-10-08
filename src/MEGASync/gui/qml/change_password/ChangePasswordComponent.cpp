#include "ChangePasswordComponent.h"

#include "ChangePasswordDialog.h"
#include "PasswordStrengthChecker.h"

static bool qmlRegistrationDone = false;

ChangePasswordComponent::ChangePasswordComponent(QObject* parent):
    QMLComponent(parent),
    mChangePasswordController(std::make_unique<ChangePasswordController>())
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(
        QString::fromLatin1("changePasswordComponentAccess"),
        this);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::show2FA,
            this,
            &ChangePasswordComponent::show2FA);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::passwordChangeFailed,
            this,
            &ChangePasswordComponent::passwordChangeFailed);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::passwordChangeSucceed,
            this,
            &ChangePasswordComponent::passwordChangeSucceed);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::twoFAVerificationFailed,
            this,
            &ChangePasswordComponent::twoFAVerificationFailed);
}

QUrl ChangePasswordComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/change_password/ChangePassword.qml"));
}

void ChangePasswordComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("ChangePasswordComponents", 1, 0);
        qmlRegisterType<ChangePasswordDialog>("ChangePasswordComponents",
                                              1,
                                              0,
                                              "ChangePasswordDialog");

        qmlRegisterType<PasswordStrengthChecker>("PasswordStrengthChecker",
                                                 1,
                                                 0,
                                                 "PasswordStrengthChecker");

        qmlRegistrationDone = true;
    }
}

void ChangePasswordComponent::changePassword(QString password, QString confirmationPassword)
{
    mChangePasswordController->changePassword(password, confirmationPassword);
}

void ChangePasswordComponent::check2FA(QString pin)
{
    mChangePasswordController->check2FA(pin);
}
