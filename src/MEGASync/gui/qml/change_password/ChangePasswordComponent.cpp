#include "ChangePasswordComponent.h"

#include "ChangePasswordDialog.h"
#include "MessageDialogData.h"
#include "MessageDialogOpener.h"
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
            &ChangePasswordComponent::onPasswordChangeFailed);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::passwordChangeSucceed,
            this,
            &ChangePasswordComponent::onPasswordChangeSucceed);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::twoFAVerificationFailed,
            this,
            &ChangePasswordComponent::twoFAVerificationFailed);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::passwordCheckFailed,
            this,
            &ChangePasswordComponent::onPasswordCheckFailed);
}

void ChangePasswordComponent::onPasswordCheckFailed(QString errorMessage)
{
    MessageDialogInfo info;
    info.descriptionText = errorMessage;

    MessageDialogOpener::critical(info);
}

void ChangePasswordComponent::onPasswordChangeFailed(QString errorMessage)
{
    emit passwordChangeFailed();

    MessageDialogInfo info;
    info.descriptionText = errorMessage;

    MessageDialogOpener::critical(info);
}

void ChangePasswordComponent::onPasswordChangeSucceed(QString title, QString description)
{
    emit passwordChangeSucceed();

    MessageDialogInfo info;
    info.titleText = title;
    info.descriptionText = description;

    MessageDialogOpener::information(info);
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
