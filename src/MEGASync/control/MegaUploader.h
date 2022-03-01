#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include "Preferences.h"
#include "megaapi.h"
#include "QTMegaRequestListener.h"

class MegaUploader : public QObject
{
    Q_OBJECT

public:
    MegaUploader(mega::MegaApi *megaApi);
    virtual ~MegaUploader();
    void upload(QString path, mega::MegaNode *parent, unsigned long long appDataID);
    bool filesdiffer(QFileInfo &source, QFileInfo &destination);

protected:
    void upload(QFileInfo info, mega::MegaNode *parent, unsigned long long appDataID);

    mega::MegaApi *megaApi;
};

#endif // MEGAUPLOADER_H
