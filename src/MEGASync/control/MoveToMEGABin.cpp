#include "MoveToMEGABin.h"

#include <MEGAPathCreator.h>
#include <MegaApiSynchronizedRequest.h>
#include <MegaApplication.h>

#include <QDate>

//Move to BIN
/// This method is run synchronously. So, it waits for the SDK to create the folders, and only then we move the files
/// As the SDK createFolder is async, we need to use QEventLoop to stop the thread (the UI thread will be frozen only if you run this method in the UI thread)
/// When the createFolder returns we continue or stop the eventloop.
/// When all files are moved, we stop the eventloop and we continue
MoveToMEGABin::MoveToBinError MoveToMEGABin::moveToBin(mega::MegaHandle handle, const QString& binFolderName, bool addDateFolder)
{
    MoveToBinError error;

    auto moveLambda = [this, handle, &error](std::shared_ptr<mega::MegaNode> rubbishNode)
    {
        std::unique_ptr<mega::MegaNode> nodeToMove(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
        if(nodeToMove)
        {
            MegaApiSynchronizedRequest::runRequestLambdaWithResult(
                [this](mega::MegaNode* node, mega::MegaNode* targetNode, mega::MegaRequestListener* listener)
                { MegaSyncApp->getMegaApi()->moveNode(node, targetNode, listener); },
                MegaSyncApp->getMegaApi(),
                this,
                [handle, &error](const mega::MegaRequest&, const mega::MegaError& e)
                {
                    if(e.getErrorCode() != mega::MegaError::API_OK)
                    {
                        error.moveError = std::shared_ptr<mega::MegaError>(e.copy());
                    }
                },
                nodeToMove.get(),
                rubbishNode.get());
        }
    };

    QString folderPath(binFolderName);
    if(addDateFolder)
    {
        QString dateFolder(QDate::currentDate().toString(Qt::DateFormat::ISODate));
        folderPath = QString::fromLatin1("%1/%2").arg(folderPath, dateFolder);
    }

    MEGAPathCreator creator;
    auto folderNode = creator.mkDir(QString::fromLatin1("//bin"), folderPath, error.binFolderCreationError);
    if(folderNode)
    {
        moveLambda(folderNode);
    }

    return error;
}
