#include "ChangePasswordComponent.h"

#include "ChangePasswordDialog.h"
#include "DialogOpener.h"
#include "MessageDialogData.h"
#include "MessageDialogOpener.h"
#include "PasswordStrengthChecker.h"
#include "SettingsDialog.h"

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
            &ChangePasswordComponent::onPasswordChangeSucceed);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::twoFAVerificationFailed,
            this,
            &ChangePasswordComponent::twoFAVerificationFailed);

    connect(mChangePasswordController.get(),
            &ChangePasswordController::passwordCheckFailed,
            this,
            &ChangePasswordComponent::passwordCheckFailed);
}

void ChangePasswordComponent::onPasswordChangeSucceed(QString title,
                                                      QString description,
                                                      QString bannerText)
{
    emit passwordChangeSucceed();

    auto dialog = DialogOpener::findDialog<SettingsDialog>();

    MessageDialogInfo info;
    info.parent = dialog ? dialog->getDialog() : nullptr;
    info.titleText = title;
    info.descriptionText = description;
    info.bannerText = bannerText;
    info.buttonsText.insert(QMessageBox::Close, tr("Done"));

    MessageDialogOpener::success(info);
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

void ChangePasswordComponent::changePassword(QString password)
{
    mChangePasswordController->changePassword(password);
}

void ChangePasswordComponent::check2FA(QString pin)
{
    mChangePasswordController->check2FA(pin);
}
