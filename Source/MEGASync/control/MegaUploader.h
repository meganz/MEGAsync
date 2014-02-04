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
    void upload(QString path, MegaNode *parent);
    virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);

signals:
    void dupplicateUpload(QString localPath, QString name, long long handle);

protected:
    void upload(QFileInfo info, MegaNode *parent);

    MegaApi *megaApi;
    QTMegaRequestListener delegateListener;
    QQueue<QFileInfo> folders;
};

#endif // MEGAUPLOADER_H
