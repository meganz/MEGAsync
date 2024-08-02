#ifndef DEVICEDATA_H
#define DEVICEDATA_H

#include "SyncStatus.h"

#include <QObject>
#include <QQmlComponent>

namespace DeviceOs
{
Q_NAMESPACE

enum Os
{
    UNDEFINED,
    LINUX,
    MAC,
    WINDOWS
};
Q_ENUM_NS(Os)
}

class DeviceData
{
    Q_GADGET

public:
    QString name;
    DeviceOs::Os os = DeviceOs::UNDEFINED;
    SyncStatus::Value status = SyncStatus::UP_TO_DATE;
    int folderCount = 0;
    qint64 totalSize = 0;

    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(DeviceOs::Os os MEMBER os)
    Q_PROPERTY(SyncStatus::Value status MEMBER status)
    Q_PROPERTY(int folderCount MEMBER folderCount)
    Q_PROPERTY(qint64 totalSize MEMBER totalSize)

    void setOsFromAgentString(const QString& agentStr)
    {
        if (agentStr.contains(QString::fromLatin1("Windows")))
        {
            os = DeviceOs::WINDOWS;
        }
        else if (agentStr.contains(QString::fromLatin1("Linux")))
        {
            os = DeviceOs::LINUX;
        }
        else
        {
            os = DeviceOs::MAC;
        }
    }
};
Q_DECLARE_METATYPE(DeviceData)

#endif // DEVICEDATA_H
