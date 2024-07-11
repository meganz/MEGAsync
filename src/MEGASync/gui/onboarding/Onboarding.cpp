#include "Onboarding.h"

#include "PasswordStrengthChecker.h"
#include "AccountStatusController.h"
#include "OnboardingQmlDialog.h"

#include "backups/Backups.h"
#include "syncs/SyncsComponent.h"

#include "SettingsDialog.h"
#include "MegaApplication.h"

#include <QQmlEngine>

using namespace mega;

Onboarding::Onboarding(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("Onboarding", 1, 0);
    qmlRegisterType<OnboardingQmlDialog>("OnboardingQmlDialog", 1, 0, "OnboardingQmlDialog");
    qmlRegisterType<AccountStatusController>("AccountStatusController", 1, 0, "AccountStatusController");
    qmlRegisterType<PasswordStrengthChecker>("PasswordStrengthChecker", 1, 0, "PasswordStrengthChecker");
    qmlRegisterUncreatableType<SettingsDialog>("SettingsDialog", 1, 0, "SettingsDialog",
                                               QString::fromUtf8("Warning SettingsDialog : not allowed to be instantiated"));

    Backups::registerQmlModules();
    SyncsComponent::registerQmlModules();

    // Makes the Guest window transparent (macOS)
    QQuickWindow::setDefaultAlphaBuffer(true);
}

QUrl Onboarding::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/onboard/OnboardingDialog.qml"));
}

QString Onboarding::contextName()
{
    return QString::fromUtf8("onboardingAccess");
}

void Onboarding::openPreferences(int tabIndex) const
{
    MegaSyncApp->openSettings(tabIndex);
}
