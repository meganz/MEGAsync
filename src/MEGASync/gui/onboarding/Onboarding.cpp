#include "Onboarding.h"

#include "AccountStatusController.h"
#include "BackupCandidatesComponent.h"
#include "MegaApplication.h"
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

bool Onboarding::deviceNameAlreadyExists(const QString& name) const
{
    // TODO : SAT-1645 implement this
    return false;
}
