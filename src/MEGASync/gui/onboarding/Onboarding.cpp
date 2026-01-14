#include "Onboarding.h"

#include "AccountStatusController.h"
#include "BackupCandidatesComponent.h"
#include "DeviceNameChecker.h"
#include "LoginController.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "OnboardingQmlDialog.h"
#include "PasswordStrengthChecker.h"
#include "SettingsDialog.h"
#include "Syncs.h"
#include "SyncsComponent.h"
#include "SyncsData.h"

#include <QQmlEngine>

using namespace mega;

Onboarding::Onboarding(QObject* parent):
    QMLComponent(parent),
    mSyncsComponent(std::make_unique<SyncsComponent>())
{
    mSyncsComponent->setSyncOrigin(SyncInfo::SyncOrigin::ONBOARDING_ORIGIN);

    qmlRegisterModule("Onboarding", 1, 0);
    qmlRegisterType<OnboardingQmlDialog>("OnboardingQmlDialog", 1, 0, "OnboardingQmlDialog");
    qmlRegisterType<AccountStatusController>("AccountStatusController", 1, 0, "AccountStatusController");
    qmlRegisterType<PasswordStrengthChecker>("PasswordStrengthChecker", 1, 0, "PasswordStrengthChecker");
    qmlRegisterUncreatableType<SettingsDialog>("SettingsDialog", 1, 0, "SettingsDialog",
                                               QString::fromUtf8("Warning SettingsDialog : not allowed to be instantiated"));

    BackupCandidatesComponent::registerQmlModules();

    // Makes the Guest window transparent (macOS)
    QQuickWindow::setDefaultAlphaBuffer(true);
}

QUrl Onboarding::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/onboard/OnboardingDialog.qml"));
}

void Onboarding::openPreferences(int tabIndex) const
{
    MegaSyncApp->openSettings(tabIndex);
}

void Onboarding::checkDeviceName(const QString& name)
{
    DeviceNameChecker* deviceNameChecker = new DeviceNameChecker(this, name);
    QObject::connect(deviceNameChecker,
                     &QThread::finished,
                     deviceNameChecker,
                     &QObject::deleteLater);

    QObject::connect(deviceNameChecker,
                     &DeviceNameChecker::deviceNameCheck,
                     this,
                     [this](bool isValid)
                     {
                         emit deviceNameChecked(isValid);
                     });

    deviceNameChecker->start();
}

void Onboarding::showClosingButLoggingInWarningDialog() const
{
    MessageDialogInfo msgInfo;
    msgInfo.titleText = QCoreApplication::translate("OnboardingStrings", "Stop logging in?");
    msgInfo.descriptionText =
        QCoreApplication::translate("OnboardingStrings",
                                    "Closing this window will stop you logging in.");
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok,
                         QCoreApplication::translate("OnboardingStrings", "Don’t stop"));
    textsByButton.insert(QMessageBox::Cancel,
                         QCoreApplication::translate("OnboardingStrings", "Stop Loggin in"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.defaultButton = QMessageBox::Cancel;
    msgInfo.finishFunc = [](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Cancel)
        {
            MegaSyncApp->getLoginController()->cancelLogin();
        }
    };
    MessageDialogOpener::warning(msgInfo);
}

void Onboarding::showClosingButCreatingAccount() const
{
    MessageDialogInfo msgInfo;
    msgInfo.titleText =
        QCoreApplication::translate("OnboardingStrings", "Cancel account creation?");
    msgInfo.descriptionText =
        QCoreApplication::translate("OnboardingStrings",
                                    "Closing this window will cancel the sign up process.");
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok,
                         QCoreApplication::translate("OnboardingStrings", "Don’t cancel"));
    textsByButton.insert(QMessageBox::Cancel,
                         QCoreApplication::translate("OnboardingStrings", "Cancel account"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.defaultButton = QMessageBox::Cancel;
    msgInfo.finishFunc = [](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Cancel)
        {
            MegaSyncApp->getLoginController()->cancelCreateAccount();
        }
    };
    MessageDialogOpener::warning(msgInfo);
}
