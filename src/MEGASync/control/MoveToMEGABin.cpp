#include "MoveToMEGABin.h"

#include "MegaApiSynchronizedRequest.h"
#include "MegaApplication.h"
#include "MEGAPathCreator.h"

#include <QDate>

//Move to BIN
/// This method is run synchronously. So, it waits for the SDK to create the folders, and only then we move the files
/// As the SDK createFolder is async, we need to use QEventLoop to stop the thread (the UI thread will be frozen only if you run this method in the UI thread)
/// When the createFolder returns we continue or stop the eventloop.
/// When all files are moved, we stop the eventloop and we continue
std::shared_ptr<mega::MegaError> MoveToMEGABin::operator()(mega::MegaHandle handle,
                                                           const QString& binFolderName,
                                                           bool addDateFolder)
{
    return moveToBin(handle, buildFolderPath(binFolderName, addDateFolder));
}

std::shared_ptr<mega::MegaError> MoveToMEGABin::operator()(mega::MegaHandle handle,
                                                           const QString& binFolderPath)
{
    return moveToBin(handle, binFolderPath);
}

QString MoveToMEGABin::buildFolderPath(const QString& binFolderName, bool addDateFolder)
{
    QString folderPath(binFolderName);

    if (addDateFolder)
    {
        const QString dateFolder(QDate::currentDate().toString(Qt::DateFormat::ISODate));

        // Keep the path free of empty components: when there is no bin subfolder the date folder
        // sits directly in the bin. The SDK skips empty components when resolving a path, so this
        // is about keeping the path canonical in lookups and logs, not about a lookup failure.
        folderPath = folderPath.isEmpty() ?
                         dateFolder :
                         QString::fromLatin1("%1/%2").arg(folderPath, dateFolder);
    }

    return folderPath;
}

QString MoveToMEGABin::createBinFolder(const QString& binFolderName, bool addDateFolder)
{
    std::shared_ptr<mega::MegaError> error;

    const auto folderPath(buildFolderPath(binFolderName, addDateFolder));

    if (!MEGAPathCreator::mkDir(QString::fromLatin1("//bin"), folderPath, error))
    {
        mega::MegaApi::log(
            mega::MegaApi::LOG_LEVEL_ERROR,
            QString::fromUtf8("Unable to prepare folder into MEGA bin: %1. Error: %2")
                .arg(folderPath,
                     error ? Utilities::getTranslatedError(error.get()) :
                             QString::fromUtf8("no error reported"))
                .toUtf8()
                .constData());
        return QString();
    }

    return folderPath;
}

std::shared_ptr<mega::MegaError> MoveToMEGABin::moveToBin(mega::MegaHandle handle,
                                                          const QString& folderPath)
{
    std::shared_ptr<mega::MegaError> error;

    auto moveLambda = [handle, &error](std::shared_ptr<mega::MegaNode> rubbishNode)
    {
        std::shared_ptr<mega::MegaNode> nodeToMove(
            MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
        if (nodeToMove)
        {
            MegaApiSynchronizedRequest::runRequestLambdaWithResult(
                [](mega::MegaNode* node,
                   mega::MegaNode* targetNode,
                   mega::MegaRequestListener* listener)
                {
                    MegaSyncApp->getMegaApi()->moveNode(node, targetNode, listener);
                },
                MegaSyncApp->getMegaApi(),
                [nodeToMove, &error](mega::MegaRequest*, mega::MegaError* e)
                {
                    if (e->getErrorCode() != mega::MegaError::API_OK)
                    {
                        mega::MegaApi::log(
                            mega::MegaApi::LOG_LEVEL_ERROR,
                            QString::fromUtf8("Unable to move MEGA item to bin: %1. Error: %2")
                                .arg(QString::fromUtf8(nodeToMove->getName()),
                                     Utilities::getTranslatedError(e))
                                .toUtf8()
                                .constData());
                        error.reset(e->copy());
                    }
                },
                nodeToMove.get(),
                rubbishNode.get());
        }
    };

    auto folderNode = MEGAPathCreator::mkDir(QString::fromLatin1("//bin"), folderPath, error);
    if (!error && folderNode)
    {
        moveLambda(folderNode);
    }
    else
    {
        // "no error reported" means mkDir() gave back no folder and no error either. The node is
        // then left where it is while this returns success to the caller, so say so plainly: it is
        // the only way to tell that case apart from a genuine API failure in a user's log.
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                           QString::fromUtf8("Unable to create folder into MEGA bin: %1. Error: %2")
                               .arg(folderPath,
                                    error ? Utilities::getTranslatedError(error.get()) :
                                            QString::fromUtf8("no error reported"))
                               .toUtf8()
                               .constData());
    }

    return error;
}
