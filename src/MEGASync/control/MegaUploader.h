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
    bool uploadRecursivelyIntoASyncedLocation(QFileInfo srcPath, QString destPath, mega::MegaNode *parent, unsigned long long appDataID);

protected:
    void upload(QFileInfo info, mega::MegaNode *parent, unsigned long long appDataID);

private:
    void startUpload(const QString& name, const QString& localPath, mega::MegaNode* parent, const unsigned long long appDataID);

    mega::MegaApi *megaApi;
};

#endif // MEGAUPLOADER_H
