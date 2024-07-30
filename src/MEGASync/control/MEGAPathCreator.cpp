#include "MEGAPathCreator.h"

#include <MegaApplication.h>
#include <MegaApiSynchronizedRequest.h>

//Create Folders
std::shared_ptr<mega::MegaNode> MEGAPathCreator::createFolder(mega::MegaNode* parentNode,
                                                              const QString& folderName,
                                                              std::shared_ptr<mega::MegaError>& error)
{
    std::shared_ptr<mega::MegaNode> node(
        MegaSyncApp->getMegaApi()->getChildNode(parentNode, folderName.toUtf8().constData()));
    if (!node)
    {
        MegaApiSynchronizedRequest::runRequestWithResult(
            &mega::MegaApi::createFolder,
            MegaSyncApp->getMegaApi(),
            [&node, &error](mega::MegaRequest* request, mega::MegaError* e)
            {
                if (e->getErrorCode() == mega::MegaError::API_OK)
                {
                    node.reset(MegaSyncApp->getMegaApi()->getNodeByHandle(request->getNodeHandle()));
                }
                else
                {
                    error.reset(e->copy());
                }
            },
            folderName.toUtf8().constData(),
            parentNode);
    }

    return node;
}

std::shared_ptr<mega::MegaNode> MEGAPathCreator::mkDir(const QString& root,
                                                       const QString& path,
                                                       std::shared_ptr<mega::MegaError>& error)
{
    auto fullPath(QLatin1String("%1/%2").arg(root, path));
    std::shared_ptr<mega::MegaNode> targetNode(MegaSyncApp->getMegaApi()->getNodeByPath(fullPath.toUtf8().constData()));
    if (targetNode)
    {
        return targetNode;
    }

    std::shared_ptr<mega::MegaNode> nodeCreated(root.isEmpty() ? MegaSyncApp->getMegaApi()->getRootNode() :
                                           MegaSyncApp->getMegaApi()->getNodeByPath(root.toUtf8().constData()));

    auto pathSplitted = path.split(QLatin1String("/"));

    while (!pathSplitted.isEmpty())
    {
        const auto followingPath(pathSplitted.takeFirst());
        if (followingPath.isEmpty())
        {
            continue;
        }

        nodeCreated = createFolder(nodeCreated.get(), followingPath, error);
        if (!nodeCreated)
        {
            pathSplitted.clear();
        }
    }

    return nodeCreated;
}
