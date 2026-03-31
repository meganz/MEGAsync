#include "GuestContent.h"

#include "GuestQmlDialog.h"
#include "MegaApplication.h"
#include "DialogOpener.h"
#include "Onboarding.h"
#include "QmlDialogManager.h"
#include "QmlDialogWrapper.h"

#include <QApplication>
#include <QQmlEngine>
#include <QTimer>

GuestContent::GuestContent(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("GuestContent", 1, 0);
    qmlRegisterType<GuestQmlDialog>("GuestQmlDialog", 1, 0, "GuestQmlDialog");
}

void GuestContent::onInitialPageButtonClicked()
{
#ifndef WIN32
    DialogOpener::removeDialogByClass<QmlDialogWrapper<GuestContent>>();
    qApp->processEvents();

    // After logout the previous onboarding wrapper can remain around in a hidden state.
    // Remove it completely before creating a fresh one, otherwise the close lifecycle can
    // race the new dialog and make it disappear immediately.
    DialogOpener::removeDialogByClass<QmlDialogWrapper<Onboarding>>();
    qApp->processEvents();

    QTimer::singleShot(
        150,
        []()
        {
            QmlDialogManager::instance()->openOnboardingDialog(true);
            QmlDialogManager::instance()->raiseOnboardingDialog();
        });
#endif
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
    return QUrl(QString::fromUtf8("qrc:/guest/GuestDialog.qml"));
}
