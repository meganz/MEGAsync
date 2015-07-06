#include "MegaDownloader.h"
#include "Utilities.h"
#include <QApplication>
#include <QDateTime>

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

void MegaDownloader::processDownloadQueue(QQueue<MegaNode *> *downloadQueue, QString path)
{
    QDir dir(path);
    if(!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
    {
        qDeleteAll(*downloadQueue);
        downloadQueue->clear();
        return;
    }

    QString currentPath;
    while(!downloadQueue->isEmpty())
    {
        MegaNode *node = downloadQueue->dequeue();

        if(node->getAuth()->size() && pathMap.contains(node->getParentHandle()))
        {
            currentPath = pathMap[node->getParentHandle()];
        }
        else
        {
            currentPath = path;
        }

        download(node, currentPath);
        delete node;
    }
    pathMap.clear();
}

void MegaDownloader::download(MegaNode *parent, QFileInfo info)
{
    QApplication::processEvents();

    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());

    if(parent->getType() == MegaNode::TYPE_FILE)
    {
        QDir dir(currentPath);
        QString fullPath = dir.filePath(QString::fromUtf8(megaApi->escapeFsIncompatible(parent->getName())));
        QFileInfo info(fullPath);
        if(info.exists())
        {
            const char *fpLocal = megaApi->getFingerprint(fullPath.toUtf8().constData());
            const char *fpRemote = megaApi->getFingerprint(parent);

            if((fpLocal && fpRemote && !strcmp(fpLocal,fpRemote))
                    || (!fpRemote && parent->getSize() == info.size()
                        && parent->getModificationTime() == (info.lastModified().toMSecsSinceEpoch()/1000)))
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

        megaApi->startDownload(parent, (currentPath + QDir::separator()).toUtf8().constData());
    }
    else
    {
        QString nodeName = QString::fromUtf8(megaApi->escapeFsIncompatible(parent->getName()));
        QString destPath = currentPath + QDir::separator() + nodeName;
        QDir dir(destPath);
        if(!dir.exists())
        {
            dir.mkpath(QString::fromAscii("."));
        }

        if(!parent->getAuth()->size())
        {
            MegaNodeList *nList = megaApi->getChildren(parent);
            for(int i=0;i<nList->size();i++)
            {
                MegaNode *child = nList->get(i);
                download(child, destPath);
            }
        }
        else
        {
            pathMap[parent->getHandle()] = destPath;
        }
    }
}
