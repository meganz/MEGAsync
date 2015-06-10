#include "MegaDownloader.h"
#include <QApplication>

using namespace mega;

MegaDownloader::MegaDownloader(MegaApi *megaApi) : QObject()
{
    this->megaApi = megaApi;
}

MegaDownloader::~MegaDownloader()
{

}

void MegaDownloader::download(MegaNode *parent, QString path)
{
    return download(parent, QFileInfo(path));
}

void MegaDownloader::download(MegaNode *parent, QFileInfo info)
{
    QApplication::processEvents();

    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());

    if(parent->getType() == MegaNode::TYPE_FILE)
    {
        QDir dir(currentPath);
        QString fullPath = dir.filePath(QString::fromUtf8(parent->getName()));
        if(QFileInfo(fullPath).exists())
        {
            const char *fpLocal = megaApi->getFingerprint(fullPath.toUtf8().constData());
            const char *fpRemote = megaApi->getFingerprint(parent);

            if(fpLocal && fpRemote && !strcmp(fpLocal,fpRemote))
            {
                delete [] fpLocal;
                delete [] fpRemote;
                emit dupplicateDownload(QDir::toNativeSeparators(fullPath),
                                        QString::fromUtf8(parent->getName()),
                                        parent->getHandle());
                return;
            }
            delete [] fpLocal;
            delete [] fpRemote;
        }

        megaApi->startDownload(parent,currentPath.toUtf8().constData());
    }
    else if(parent->getType() == MegaNode::TYPE_FOLDER)
    {
        QString destPath = currentPath + QDir::separator() + QString::fromUtf8(parent->getName());
        QDir dir(destPath);
        if(!dir.exists()) dir.mkpath(QString::fromAscii("."));
        MegaNodeList *nList = megaApi->getChildren(parent);
        for(int i=0;i<nList->size();i++)
        {
            MegaNode *child = nList->get(i);
            download(child,destPath+QDir::separator());
        }
    }
}
