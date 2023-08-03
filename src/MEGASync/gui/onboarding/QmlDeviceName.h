#ifndef QMLDEVICENAME_H
#define QMLDEVICENAME_H

#include "UserAttributesRequests/DeviceName.h"

#include <QObject>
#include <memory>

class QmlDeviceName : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mName READ getDeviceName WRITE setDeviceName NOTIFY deviceNameChanged)

public:
    explicit QmlDeviceName(QObject *parent = nullptr);

    Q_INVOKABLE QString getDeviceName();
    Q_INVOKABLE bool setDeviceName(const QString& newName);

signals:
    void deviceNameChanged();
    void deviceNameSet();

private slots:
    void onDeviceNameSet();

private:
    QString mName;
    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
    bool mChanging;
    bool mRequesting;
};

#endif // QMLDEVICENAME_H
