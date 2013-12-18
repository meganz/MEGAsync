#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include "Preferences.h"
#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaRequestListener.h"

class MegaUploader : public MegaRequestListener
{
public:
    MegaUploader(MegaApi *megaApi, Preferences *preferences);
    bool upload(QString path, Node *parent);
    virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);

protected:
    bool upload(QFileInfo info, Node *parent);

    MegaApi *megaApi;
    Preferences *preferences;
    QTMegaRequestListener delegateListener;
    QQueue<QFileInfo> folders;
};

#endif // MEGAUPLOADER_H
