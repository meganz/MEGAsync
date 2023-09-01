#include "GuestContent.h"
#include "MegaApplication.h"
#include "GuestQmlDialog.h"

#include <QQmlEngine>

GuestContent::GuestContent(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("Guest", 1, 0);
    qmlRegisterModule("GuestContent", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/guest/GuestItem.qml")), "Guest", 1, 0, "GuestItem");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/content/guest/GuestStrings.qml")), "Guest", 1, 0, "GuestStrings");
    qmlRegisterType<GuestQmlDialog>("GuestQmlDialog", 1, 0, "GuestQmlDialog");
}

GuestContent::~GuestContent()
{
}

void GuestContent::onAboutMEGAClicked()
{
    MegaSyncApp->onAboutClicked();
}

void GuestContent::onPreferencesClicked()
{
    MegaSyncApp->openSettings();
}

void GuestContent::onExitClicked()
{
    MegaSyncApp->tryExitApplication();
}

void GuestContent::onVerifyEmailClicked()
{
    MegaSyncApp->getMegaApi()->resendVerificationEmail();
}

void GuestContent::onLogoutClicked()
{
    MegaSyncApp->unlink();
}

QUrl GuestContent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/content/guest/GuestDialog.qml"));
}

QString GuestContent::contextName()
{
    return QString::fromUtf8("GuestContent");
}
