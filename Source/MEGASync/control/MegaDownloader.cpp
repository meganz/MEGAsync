#include "MegaDownloader.h"

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
    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());

    if(parent->getType() == MegaNode::TYPE_FILE)
    {
        megaApi->startDownload(parent,currentPath.toUtf8().constData());
    }
    else if(parent->getType() == MegaNode::TYPE_FOLDER)
    {
        QString destPath = currentPath + QDir::separator() + QString::fromUtf8(parent->getName());
        QDir dir(destPath);
        if(!dir.exists()) dir.mkpath(QString::fromAscii("."));
        NodeList *nList = megaApi->getChildren(parent);
        for(int i=0;i<nList->size();i++)
        {
            MegaNode *child = nList->get(i);
            download(child,destPath+QDir::separator());
        }

    }
}
