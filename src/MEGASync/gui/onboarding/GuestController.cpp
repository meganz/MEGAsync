#include "GuestController.h"
#include "MegaApplication.h"
#include "GuestWindow.h"

#include <QQmlEngine>

GuestController::GuestController(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("Guest", 1, 0);
    qmlRegisterModule("GuestController", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/guest/GuestContent.qml")), "Guest", 1, 0, "GuestContent");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/content/guest/GuestStrings.qml")), "Guest", 1, 0, "GuestStrings");
    qmlRegisterType<GuestWindow>("GuestWindow", 1, 0, "GuestWindow");
}

void GuestController::onAboutMEGAClicked()
{
    MegaSyncApp->onAboutClicked();
}

void GuestController::onPreferencesClicked()
{
    MegaSyncApp->openSettings();
}

void GuestController::onExitClicked()
{
    MegaSyncApp->tryExitApplication();
}

QUrl GuestController::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/content/guest/GuestDialog.qml"));
}

QString GuestController::contextName()
{
    return QString::fromUtf8("GuestController");
}
