#include "Backups.h"

#include "BackupsQmlDialog.h"

Backups::Backups(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("Backups", 1, 0);
    qmlRegisterType<BackupsQmlDialog>("BackupsQmlDialog", 1, 0, "BackupsQmlDialog");
}

QUrl Backups::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/backups/BackupsDialog.qml"));
}

QString Backups::contextName()
{
    return QString::fromUtf8("backupsAccess");
}
