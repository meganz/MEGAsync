#include "GuestContent.h"
#include "MegaApplication.h"
#include "GuestQmlDialog.h"

#include <QQmlEngine>

GuestContent::GuestContent(QObject *parent)
    : QMLComponent(parent)
{
    //getEngine()->addImportPath(QString::fromUtf8("qrc:/guest"));
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

void GuestContent::onVerifyPhoneClicked()
{
    MegaSyncApp->goToMyCloud();
}

void GuestContent::onLogoutClicked()
{
    MegaSyncApp->unlink();
}

QUrl GuestContent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:guest/GuestDialog.qml"));
}

QString GuestContent::contextName()
{
    return QString::fromUtf8("guestContentAccess");
}
