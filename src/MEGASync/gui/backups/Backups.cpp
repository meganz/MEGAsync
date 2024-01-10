#include "Backups.h"

#include "qml/QmlDialog.h"

Backups::Backups(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("Backups", 1, 0);
    qmlRegisterType<QmlDialog>("QmlDialog", 1, 0, "QmlDialog");
}

QUrl Backups::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/backups/BackupsDialog.qml"));
}

QString Backups::contextName()
{
    return QString::fromUtf8("backupsAccess");
}
