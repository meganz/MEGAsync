#include "MacXSystemServiceTask.h"
#include "MEGAService.h"

using namespace mega;
using namespace std;

MacXSystemServiceTask::MacXSystemServiceTask(MegaApplication *receiver)
{
    this->receiver = receiver;
    listener = [[MEGAService alloc] initWithDelegate:this];

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA service starting...");
    connect(this, SIGNAL(newUploadQueue(QQueue<QString>)), receiver, SLOT(shellUpload(QQueue<QString>)));
    connect(this, SIGNAL(newExportQueue(QQueue<QString>)), receiver, SLOT(shellExport(QQueue<QString>)));

}

void MacXSystemServiceTask::processItems(QStringList itemsSelected)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA service processing items...");

    MegaApplication *app = (MegaApplication *)qApp;
    MegaApi *megaApi = app->getMegaApi();

    for(int i = 0; i < itemsSelected.size(); i++)
    {

        QFileInfo file(itemsSelected.at(i));
        if (file.exists())
        {
            string tmpPath(itemsSelected.at(i).toUtf8().constData());
            MegaNode *node = megaApi->getSyncedNode(&tmpPath);
            if (!node)
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA service not synced node...");
                const char *fpLocal = megaApi->getFingerprint(itemsSelected.at(i).toUtf8().constData());
                node = megaApi->getExportableNodeByFingerprint(fpLocal);
                delete [] fpLocal;
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA service found exportable node...");
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA service synced node...");
            }

            if(node)
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA service enqueue export link...");
                exportQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
                delete node;
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA service enqueue upload item...");
                uploadQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
            }

        }
    }

    if(!exportQueue.isEmpty())
    {
        emit newExportQueue(exportQueue);
        exportQueue.clear();
    }

    if(!uploadQueue.isEmpty())
    {
        emit newUploadQueue(uploadQueue);
        uploadQueue.clear();
    }
}

MacXSystemServiceTask::~MacXSystemServiceTask()
{

}

