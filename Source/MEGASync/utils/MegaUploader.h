#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include "Preferences.h"
#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaRequestListener.h"

class MegaUploader : public QObject, public MegaRequestListener
{
    Q_OBJECT

public:
    MegaUploader(MegaApi *megaApi);
    bool upload(QString path, Node *parent);
    virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);

signals:
    void startFileCopy();

protected:
    bool upload(QFileInfo info, Node *parent);

    MegaApi *megaApi;
    QTMegaRequestListener delegateListener;
    QQueue<QFileInfo> folders;
};

#endif // MEGAUPLOADER_H
