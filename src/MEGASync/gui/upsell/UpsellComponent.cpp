#include "UpsellComponent.h"

static bool qmlRegistrationDone = false;

UpsellComponent::UpsellComponent(QObject* parent):
    QMLComponent(parent)
{
    registerQmlModules();
}

QUrl UpsellComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/upsell/UpsellDialog.qml"));
}

QString UpsellComponent::contextName()
{
    return QString::fromUtf8("upsellComponentAccess");
}

void UpsellComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("Upsell", 1, 0);
        qmlRegistrationDone = true;
    }
}
