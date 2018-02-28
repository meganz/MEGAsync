#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include "Preferences.h"
#include "megaapi.h"
#include "QTMegaRequestListener.h"

class MegaUploader
{

public:
    MegaUploader(mega::MegaApi *megaApi);
    virtual ~MegaUploader();
    void upload(QString path, mega::MegaNode *parent, unsigned long long appDataID = 0);

protected:
    void upload(QFileInfo info, mega::MegaNode *parent, unsigned long long appDataID = 0);

    mega::MegaApi *megaApi;
};

#endif // MEGAUPLOADER_H
