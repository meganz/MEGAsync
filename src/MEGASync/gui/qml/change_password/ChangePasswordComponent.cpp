#include "ChangePasswordComponent.h"

#include "ChangePasswordDialog.h"

static bool qmlRegistrationDone = false;

ChangePasswordComponent::ChangePasswordComponent(QObject* parent):
    QMLComponent(parent)
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(
        QString::fromLatin1("changePasswordComponentAccess"),
        this);
}

QUrl ChangePasswordComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/change_password/ChangePassword.qml"));
}

void ChangePasswordComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("ChangePasswordComponents", 1, 0);
        qmlRegisterType<ChangePasswordDialog>("ChangePasswordComponents",
                                              1,
                                              0,
                                              "ChangePasswordDialog");

        qmlRegistrationDone = true;
    }
}

void ChangePasswordComponent::changePassword(QString password) {}
