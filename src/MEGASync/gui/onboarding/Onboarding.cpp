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
    mSyncs(std::make_unique<Syncs>())
{
    qmlRegisterModule("Onboarding", 1, 0);
    qmlRegisterType<OnboardingQmlDialog>("OnboardingQmlDialog", 1, 0, "OnboardingQmlDialog");
    qmlRegisterType<AccountStatusController>("AccountStatusController", 1, 0, "AccountStatusController");
    qmlRegisterType<PasswordStrengthChecker>("PasswordStrengthChecker", 1, 0, "PasswordStrengthChecker");
    qmlRegisterUncreatableType<SettingsDialog>("SettingsDialog", 1, 0, "SettingsDialog",
                                               QString::fromUtf8("Warning SettingsDialog : not allowed to be instantiated"));

    SyncsComponent::registerQmlModules();
    BackupCandidatesComponent::registerQmlModules();

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsComponentAccess"),
                                                   this);

    auto syncsData = mSyncs->getSyncsData();
    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsData"), syncsData);

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

/*
 * public slots to be called from qml code
 */
void Onboarding::addSync(SyncInfo::SyncOrigin origin, const QString& local, const QString& remote)
{
    mSyncs->addSync(origin, local, remote);
}

bool Onboarding::checkLocalSync(const QString& path)
{
    return mSyncs->checkLocalSync(path);
}

bool Onboarding::checkRemoteSync(const QString& path)
{
    return mSyncs->checkRemoteSync(path);
}

void Onboarding::clearRemoteError()
{
    mSyncs->clearRemoteError();
}

void Onboarding::clearLocalError()
{
    mSyncs->clearLocalError();
}
