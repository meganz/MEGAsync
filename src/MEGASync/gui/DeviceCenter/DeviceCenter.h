#ifndef DEVICE_CENTER_H
#define DEVICE_CENTER_H

#include "QmlDialogWrapper.h"

class DeviceCenter: public QMLComponent
{
    Q_OBJECT

public:
    explicit DeviceCenter(QObject* parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;
    static void registerQmlModules();
};

#endif // DEVICE_CENTER_H
