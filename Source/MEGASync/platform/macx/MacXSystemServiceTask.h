#ifndef MACXSYSTEMSERVICETASK_H
#define MACXSYSTEMSERVICETASK_H

#include <QString>
#include <objc/objc-runtime.h>
#include "MegaApplication.h"


class MacXSystemServiceTask : public QObject
{
    Q_OBJECT

public:
    MacXSystemServiceTask(MegaApplication *receiver);
    virtual ~MacXSystemServiceTask();

    void processItems(QStringList itemsSelected);

protected:
    QQueue<QString> uploadQueue;
    QQueue<QString> exportQueue;
    MegaApplication *receiver;
    id listener;

signals:
    void newUploadQueue(QQueue<QString> uploadQueue);
    void newExportQueue(QQueue<QString> exportQueue);

};
#endif // MACXSYSTEMSERVICETASK_H
