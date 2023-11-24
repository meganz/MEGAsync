#include "Onboarding.h"
#include "MegaApplication.h"

#include <QQmlEngine>
#include "Syncs.h"
#include "AccountInfoData.h"
#include "ChooseFolder.h"
#include "PasswordStrengthChecker.h"
#include "QmlDeviceName.h"
#include "AccountStatusController.h"
#include "SettingsDialog.h"
#include "BackupsModel.h"
#include "qml/QmlClipboard.h"
#include "qml/ApiEnums.h"
#include "OnboardingQmlDialog.h"

using namespace mega;

Onboarding::Onboarding(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("Onboarding", 1, 0);


    qmlRegisterType<BackupsProxyModel>("BackupsProxyModel", 1, 0, "BackupsProxyModel");

    qmlRegisterUncreatableType<BackupsModel>("BackupsModel", 1, 0, "BackupErrorCode",
                                             QString::fromUtf8("Cannot create WarningLevel in QML"));
    qmlRegisterType<OnboardingQmlDialog>("OnboardingQmlDialog", 1, 0, "OnboardingQmlDialog");
    qmlRegisterType<AccountStatusController>("AccountStatusController", 1, 0, "AccountStatusController");
    qmlRegisterType<Syncs>("Syncs", 1, 0, "Syncs");
    qmlRegisterType<PasswordStrengthChecker>("PasswordStrengthChecker", 1, 0, "PasswordStrengthChecker");
    qmlRegisterType<QmlDeviceName>("QmlDeviceName", 1, 0, "QmlDeviceName");
    qmlRegisterType<ChooseLocalFolder>("ChooseLocalFolder", 1, 0, "ChooseLocalFolder");
    qmlRegisterType<ChooseRemoteFolder>("ChooseRemoteFolder", 1, 0, "ChooseRemoteFolder");
    qmlRegisterSingletonType<AccountInfoData>("AccountInfoData", 1, 0, "AccountInfoData", AccountInfoData::instance);
    qmlRegisterUncreatableType<SettingsDialog>("SettingsDialog", 1, 0, "SettingsDialog",
                                               QString::fromUtf8("Warning SettingsDialog : not allowed to be instantiated"));

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
