#include "MergeMEGAFolders.h"

#include "MegaApiSynchronizedRequest.h"
#include "MegaApplication.h"
#include "MoveToMEGABin.h"
#include "Utilities.h"

MergeMEGAFolders::MergeMEGAFolders(ActionForDuplicates action, Qt::CaseSensitivity sensitivity):
    mCaseSensitivity(sensitivity),
    mAction(action)
{}

std::shared_ptr<mega::MegaError> MergeMEGAFolders::merge(mega::MegaNode* folderTarget,
                                                         mega::MegaNode* folderToMerge)
{
    std::shared_ptr<mega::MegaError> error(nullptr);

    if (folderTarget && folderToMerge)
    {
        error = performMerge(folderTarget, folderToMerge);
    }
    else if (folderTarget)
    {
        std::unique_ptr<mega::MegaNode> parentNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(folderTarget->getParentHandle()));
        if (parentNode)
        {
            QString targetNodeName(getNodeName(folderTarget));

            std::unique_ptr<mega::MegaNodeList> folderToMergeNodes(
                MegaSyncApp->getMegaApi()->getChildren(parentNode.get()));
            for (int index = 0; index < folderToMergeNodes->size(); ++index)
            {
                auto node(folderToMergeNodes->get(index));
                QString checkNodeName(getNodeName(node));
                if (targetNodeName.compare(checkNodeName) == 0 &&
                    node->getHandle() != folderTarget->getHandle())
                {
                    error = performMerge(folderTarget, node);
                    if (error)
                    {
                        break;
                    }
                }
            }
        }
    }

    logError(error);

    return error;
}

std::shared_ptr<mega::MegaError> MergeMEGAFolders::performMerge(mega::MegaNode* folderTarget,
                                                                mega::MegaNode* folderToMerge)
{
    // Fill the folderTarget child names container, used to know if the folderToMerge nested nodes
    // will be moved, rename or removed
    QMap<QString, std::shared_ptr<mega::MegaNode>> targetNodeWithNameConflict;
    QMap<QString, std::shared_ptr<mega::MegaNode>> targetNodeWithoutNameConflict;

    readTargetFolder(folderTarget, targetNodeWithoutNameConflict, targetNodeWithNameConflict);

    std::shared_ptr<mega::MegaError> error =
        fixTargetFolderNameConflicts(targetNodeWithNameConflict);

    if (error)
    {
        return error;
    }

    error = mergeNestedNodesIntoTargetFolder(folderTarget,
                                             folderToMerge,
                                             targetNodeWithoutNameConflict);

    if (error)
    {
        return error;
    }

    return finishMerge(folderTarget, folderToMerge);
}

void MergeMEGAFolders::readTargetFolder(
    mega::MegaNode* folderTarget,
    QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithoutNameConflict,
    QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithNameConflict)
{
    std::unique_ptr<mega::MegaNodeList> folderTargetNodes(
        MegaSyncApp->getMegaApi()->getChildren(folderTarget));

    for (int index = folderTargetNodes->size() - 1; index >= 0; --index)
    {
        auto node(folderTargetNodes->get(index));
        QString nodeName(getNodeName(node));

        // Name conflict detected
        if (!targetNodeWithoutNameConflict.contains(nodeName))
        {
            targetNodeWithoutNameConflict.insert(nodeName,
                                                 std::shared_ptr<mega::MegaNode>(node->copy()));
        }
        else if (!targetNodeWithNameConflict.contains(nodeName))
        {
            targetNodeWithNameConflict.insert(nodeName,
                                              std::shared_ptr<mega::MegaNode>(node->copy()));
        }
    }
}

std::shared_ptr<mega::MegaError> MergeMEGAFolders::fixTargetFolderNameConflicts(
    const QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithNameConflict)
{
    // Check if the target folder has name conflicts and solve them first
    if (targetNodeWithNameConflict.size() > 0)
    {
        foreach(auto node, targetNodeWithNameConflict)
        {
            auto error = merge(node.get(), nullptr);
            if (error)
            {
                return error;
            }
        }
    }

    return nullptr;
}

std::shared_ptr<mega::MegaError> MergeMEGAFolders::finishMerge(mega::MegaNode* folderTarget,
                                                               mega::MegaNode* folderToMerge)
{
    std::shared_ptr<mega::MegaError> error(nullptr);
    bool remove(false);

    if (folderToMerge->isFolder())
    {
        std::unique_ptr<mega::MegaNodeList> folderChild(
            MegaSyncApp->getMegaApi()->getChildren(folderToMerge));

        remove = folderChild->size() == 0;
    }

    if (mAction == ActionForDuplicates::IgnoreAndRemove || remove)
    {
        error = MegaApiSynchronizedRequest::runRequest(&mega::MegaApi::remove,
                                                       MegaSyncApp->getMegaApi(),
                                                       folderToMerge);
    }
    else if (mAction == ActionForDuplicates::IgnoreAndMoveToBin)
    {
        error =
            MoveToMEGABin()(folderToMerge->getHandle(), QLatin1String("FoldersMerge"), true);
    }
    else if (mAction == ActionForDuplicates::Rename)
    {
        QStringList itemsBeingRenamed;
        error = rename(folderToMerge, folderTarget, itemsBeingRenamed);
    }

    return error;
}

std::shared_ptr<mega::MegaError> MergeMEGAFolders::mergeNestedNodesIntoTargetFolder(
    mega::MegaNode* folderTarget,
    mega::MegaNode* folderToMerge,
    QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithoutNameConflict)
{
    QStringList itemsBeingRenamed;

    std::shared_ptr<mega::MegaError> error(nullptr);
    std::unique_ptr<mega::MegaNodeList> folderToMergeNodes(
        MegaSyncApp->getMegaApi()->getChildren(folderToMerge));

    for (int index = 0; index < folderToMergeNodes->size(); ++index)
    {
        auto nestedNodeToMerge(folderToMergeNodes->get(index));
        QString nestedNodeName(getNodeName(nestedNodeToMerge));
        auto targetNode(targetNodeWithoutNameConflict.value(nestedNodeName));

        // There are one item with the same name in folderTarget (if there were more, they were
        // merged in "fixTargetFolderNameConflicts" method)
        if (targetNode)
        {
            if (nestedNodeToMerge->isFile() && targetNode->isFile())
            {
                auto nodeToMoveFp(QString::fromUtf8(nestedNodeToMerge->getFingerprint()));
                auto targetNodeFp(QString::fromUtf8(targetNode->getFingerprint()));
                if (nodeToMoveFp == targetNodeFp)
                {
                    error = MegaApiSynchronizedRequest::runRequest(&mega::MegaApi::remove,
                                                                   MegaSyncApp->getMegaApi(),
                                                                   nestedNodeToMerge);
                }
                else
                {
                    error = rename(nestedNodeToMerge, folderTarget, itemsBeingRenamed);
                }
            }
            else if (nestedNodeToMerge->isFolder() && targetNode->isFolder())
            {
                error = merge(targetNode.get(), nestedNodeToMerge);
            }
            else
            {
                error = rename(nestedNodeToMerge, folderTarget, itemsBeingRenamed);
            }
        }
        // We can simply move the node, as there is no item with the same name in the target node
        else
        {
            error = MegaApiSynchronizedRequest::runRequestLambda(
                [](mega::MegaNode* node,
                   mega::MegaNode* targetNode,
                   mega::MegaRequestListener* listener)
                {
                    MegaSyncApp->getMegaApi()->moveNode(node, targetNode, listener);
                },
                MegaSyncApp->getMegaApi(),
                nestedNodeToMerge,
                folderTarget);

            // If the node was correctly moved, now it is part of the targetNodeWithoutNameConflict
            if (!error)
            {
                targetNodeWithoutNameConflict.insert(
                    nestedNodeName,
                    std::shared_ptr<mega::MegaNode>(nestedNodeToMerge->copy()));
            }
        }

        // Don´t continue if any step failed
        if (error)
        {
            break;
        }
    }

    return error;
}

void MergeMEGAFolders::logError(std::shared_ptr<mega::MegaError> error)
{
    if (error)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                           QString::fromUtf8("Merge folders failed. Error: %1")
                               .arg(Utilities::getTranslatedError(error.get()))
                               .toUtf8()
                               .constData());
    }
}

std::shared_ptr<mega::MegaError> MergeMEGAFolders::rename(mega::MegaNode* nodeToRename,
                                                          mega::MegaNode* parentNode,
                                                          QStringList& itemsBeingRenamed)
{
    QString currentName(getNodeName(nodeToRename));
    QString newName = Utilities::getNonDuplicatedNodeName(nodeToRename,
                                                          parentNode,
                                                          currentName,
                                                          true,
                                                          itemsBeingRenamed);

    auto error = MegaApiSynchronizedRequest::runRequestLambda(
        [](mega::MegaNode* node,
           mega::MegaNode* targetNode,
           const char* newName,
           mega::MegaRequestListener* listener)
        {
            MegaSyncApp->getMegaApi()->moveNode(node, targetNode, newName, listener);
        },
        MegaSyncApp->getMegaApi(),
        nodeToRename,
        parentNode,
        newName.toUtf8().constData());

    if (!error)
    {
        itemsBeingRenamed.append(newName);
    }

    return error;
}

QString MergeMEGAFolders::getNodeName(mega::MegaNode* node)
{
    QString nodeName =
        QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(node->getName()));

    if (mCaseSensitivity == Qt::CaseInsensitive)
    {
        nodeName = nodeName.toLower();
    }

    return nodeName;
}
