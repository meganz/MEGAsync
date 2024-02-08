#include "SyncsComponent.h"

#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

SyncsComponent::SyncsComponent(QObject *parent)
    : QMLComponent(parent)
{
    registerQmlModules();
}

QUrl SyncsComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/syncs/SyncsDialog.qml"));
}

QString SyncsComponent::contextName()
{
    return QString::fromUtf8("syncsAccess");
}

void SyncsComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("Syncs", 1, 0);
        qmlRegistrationDone = true;
    }
}

void SyncsComponent::openSyncsTabInPreferences() const
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
}
