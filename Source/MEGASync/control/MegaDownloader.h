#ifndef MEGADOWNLOADER_H
#define MEGADOWNLOADER_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include "megaapi.h"

class MegaDownloader : public QObject
{
    Q_OBJECT

public:
    MegaDownloader(mega::MegaApi *megaApi);
    virtual ~MegaDownloader();
    void download(mega::MegaNode *parent, QString path);

protected:
    void download(mega::MegaNode *parent, QFileInfo info);

    mega::MegaApi *megaApi;

};

#endif // MEGADOWNLOADER_H
