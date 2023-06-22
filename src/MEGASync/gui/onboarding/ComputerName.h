#ifndef COMPUTERNAME_H
#define COMPUTERNAME_H

#include "UserAttributesRequests/DeviceName.h"

#include <QObject>
#include <memory>

class ComputerName : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName MEMBER mDeviceName READ getDeviceName WRITE setDeviceName NOTIFY deviceNameChanged)

public:
    explicit ComputerName(QObject *parent = nullptr);

    Q_INVOKABLE QString getDeviceName();
    Q_INVOKABLE bool setDeviceName(const QString& newName);

signals:
    void deviceNameChanged();

private slots:
    void onDeviceNameSet();

private:
    QString mDeviceName;
    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
    bool mRequesting;
};

#endif // COMPUTERNAME_H
