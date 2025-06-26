#include "MessageDialogComponent.h"

static bool qmlRegistrationDone = false;

MessageDialogComponent::MessageDialogComponent(QObject* parent, QPointer<MessageDialogData> data):
    QMLComponent(parent),
    mData(data)
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(mData.data());
}

QUrl MessageDialogComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/messageDialogs/MessageDialog.qml"));
}

QList<QObject*> MessageDialogComponent::getInstancesFromContext()
{
    QList<QObject*> instances;
    QObject* messageDialogObject(qobject_cast<QObject*>(mData.data()));
    if (messageDialogObject)
    {
        instances.append(messageDialogObject);
    }
    return instances;
}

void MessageDialogComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterUncreatableType<MessageDialogButtonInfo>(
            "MessageDialogButtonInfo",
            1,
            0,
            "MessageDialogButtonInfo",
            QString::fromLatin1("MessageDialogButtonInfo can only be used for the enum values"));
        qmlRegisterUncreatableType<MessageDialogTextInfo>(
            "MessageDialogTextInfo",
            1,
            0,
            "MessageDialogTextInfo",
            QString::fromLatin1("MessageDialogTextInfo can only be used for the enum values"));
        qmlRegistrationDone = true;
    }
}

void MessageDialogComponent::buttonClicked(int type)
{
    mData->buttonClicked(static_cast<QMessageBox::StandardButton>(type));
}

void MessageDialogComponent::setChecked(bool checked)
{
    mData->setCheckboxChecked(checked);
}
