#include "Preferences.h"
#include "Onboarding.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include <QQmlEngine>
#include "Syncs.h"
#include "AccountInfoData.h"
#include "ChooseFolder.h"
#include "PasswordStrengthChecker.h"
#include "ComputerName.h"
#include "LoginController.h"

using namespace mega;

Onboarding::Onboarding(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("Onboard", 1, 0);
    qmlRegisterModule("Onboarding", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/OnboardingDialog.qml")), "Onboard", 1, 0, "OnboardingDialog");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/content/onboard/OnboardingStrings.qml")), "Onboard", 1, 0, "OnboardingStrings");

    qmlRegisterType<LoginController>("LoginController", 1, 0, "LoginController");
    qmlRegisterType<Syncs>("Syncs", 1, 0, "Syncs");
    qmlRegisterType<PasswordStrengthChecker>("PasswordStrengthChecker", 1, 0, "PasswordStrengthChecker");
    qmlRegisterType<ComputerName>("ComputerName", 1, 0, "ComputerName");


    qmlRegisterType<AccountInfoData>("AccountInfoData", 1, 0, "AccountInfoData");
    qmlRegisterType<ChooseLocalFolder>("ChooseLocalFolder", 1, 0, "ChooseLocalFolder");
    qmlRegisterType<ChooseRemoteFolder>("ChooseRemoteFolder", 1, 0, "ChooseRemoteFolder");

    qmlRegisterModule("Onboard.Syncs_types", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/MainFlow.qml")), "Onboard.Syncs_types", 1, 0, "MainFlow");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsFlow.qml")), "Onboard.Syncs_types", 1, 0, "SyncsFlow");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/Header.qml")), "Onboard.Syncs_types", 1, 0, "Header");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/Footer.qml")), "Onboard.Syncs_types", 1, 0, "Footer");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/InfoAccount.qml")), "Onboard.Syncs_types", 1, 0, "InfoAccount");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/ResumePage.qml")), "Onboard.Syncs_types", 1, 0, "ResumePage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsPage.qml")), "Onboard.Syncs_types", 1, 0, "SyncsPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsType.qml")), "Onboard.Syncs_types", 1, 0, "SyncsType");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsVerticalButton.qml")), "Onboard.Syncs_types", 1, 0, "SyncsVerticalButton");

    qmlRegisterModule("Onboard.Syncs_types.Syncs", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/SyncsFlow.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "SyncsFlow");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/SyncTypePage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "SyncTypePage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/FullSyncPage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "FullSyncPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/SelectiveSyncPage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "SelectiveSyncPage");

    qmlRegisterModule("Onboard.Syncs_types.Left_panel", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/left_panel/StepPanel.qml")), "Onboard.Syncs_types.Left_panel", 1, 0, "StepPanel");

    qmlRegisterModule("Onboard.Syncs_types.Backups", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/backups/BackupsFlow.qml")), "Onboard.Syncs_types.Backups", 1, 0, "BackupsFlow");

    qmlRegisterModule("BackupsModel", 1, 0);
    qmlRegisterModule("BackupsController", 1, 0);
}


QUrl Onboarding::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/main.qml"));
}

QString Onboarding::contextName()
{
    return QString::fromUtf8("Onboarding");
}

void Onboarding::openPreferences(bool sync) const
{
    int tab = SettingsDialog::BACKUP_TAB;
    if(sync)
    {
        tab = SettingsDialog::SYNCS_TAB;
    }
    MegaSyncApp->openSettings(tab);
}




