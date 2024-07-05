#include "NameConflictStalledIssue.h"

#include "mega/types.h"
#include "Utilities.h"
#include "StatsEventHandler.h"
#include "StalledIssuesModel.h"
#include "gui/NodeNameSetterDialog/RenameNodeDialog.h"

#include <MegaApiSynchronizedRequest.h>

NameConflictedStalledIssue::NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue)
{}

void NameConflictedStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    auto localConflictNames = stall->pathCount(false);

    if(localConflictNames > 0)
    {
        initLocalIssue();

        for(unsigned int index = 0; index < localConflictNames; ++index)
        {
            QFileInfo localPath(QString::fromUtf8(stall->path(false,index)));

            if(consultLocalData()->mPath.isEmpty())
            {
                getLocalData()->mPath.path = localPath.filePath();
            }

            std::shared_ptr<ConflictedNameInfo> info(new ConflictedNameInfo(localPath, localPath.isFile(), std::make_shared<LocalFileFolderAttributes>(QDir::toNativeSeparators(localPath.filePath()), nullptr)));
            mLocalConflictedNames.append(info);

            setIsFile(localPath.filePath(), true);

            //Use for autosolve
            info->mItemAttributes->initAllAttributes();
        }
    }

    auto cloudConflictNames = stall->pathCount(true);

    if(cloudConflictNames > 0)
    {
        initCloudIssue();

        auto firstCloudPath(stall->path(true,0));
        if(consultCloudData()->mPath.isEmpty())
        {
            QFileInfo cloudPathInfo(QString::fromUtf8(firstCloudPath));
            //We set the first path, as it will be used to get the folder path (discarding the filename)
            getCloudData()->mPath.path = cloudPathInfo.filePath();
            getCloudData()->mPathHandle = stall->cloudNodeHandle(0);
        }

        for(unsigned int index = 0; index < cloudConflictNames; ++index)
        {
            auto cloudHandle(stall->cloudNodeHandle(index));
            auto cloudPath = QString::fromUtf8(stall->path(true, index));
            if(cloudPath.startsWith(QLatin1String("//")))
            {
                cloudPath = cloudPath.remove(0,1);
            }

            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudHandle));
            if(node)
            {
                QFileInfo cloudPathInfo(cloudPath);
                std::shared_ptr<ConflictedNameInfo> info(new ConflictedNameInfo(cloudPathInfo, node->isFile(), std::make_shared<RemoteFileFolderAttributes>(cloudHandle, nullptr, true)));
                info->mHandle = cloudHandle;

                if(node->isFile())
                {
                    mCloudConflictedNames.addFileConflictedName(node->getModificationTime(), node->getSize(), node->getCreationTime(), QString::fromUtf8(node->getFingerprint()), info);
                    mFiles++;
                }
                else
                {
                    mCloudConflictedNames.addFolderConflictedName(cloudHandle, info);
                    mFolders++;
                }

                //Use for autosolve
                info->mItemAttributes->initAllAttributes();
            }
        }
    }
}

void NameConflictedStalledIssue::updateIssue(const mega::MegaSyncStall *stallIssue)
{
   mCloudConflictedNames.clear();
   mLocalConflictedNames.clear();

   mLocalData.reset();
   mCloudData.reset();

   fillIssue(stallIssue);
   endFillingIssue();
}

QStringList NameConflictedStalledIssue::getLocalFiles()
{
    QStringList files;
    foreach(auto nameConflict, mLocalConflictedNames)
    {
        files << nameConflict->mConflictedPath;
    }
    return files;
}

bool NameConflictedStalledIssue::shouldBeIgnored() const
{
    //We need to check if all of them are well decrypted
    //This is a temporary fix: the SDK is sending a name conflict for undecrypted nodes in shared folders
    auto areAllKeysDecrypted = mCloudConflictedNames.areAllKeysDecrypted();

    return !areAllKeysDecrypted;
}

void NameConflictedStalledIssue::showRemoteRenameHasFailedMessageBox(const mega::MegaError& error, bool isFile)
{
    QMegaMessageBox::MessageBoxInfo failedInfo;
    failedInfo.title = MegaSyncApp->getMEGAString();
    failedInfo.textFormat = Qt::RichText;
    failedInfo.buttons = QMessageBox::Yes;
    if(isFile)
    {
        failedInfo.text = StalledIssuesStrings::RemoveFileFailedTitle();
    }
    else
    {
        failedInfo.text = StalledIssuesStrings::RemoveFolderFailedTitle();
    }
    failedInfo.informativeText = StalledIssuesStrings::RemoveRemoteFailedDescription(&error);

    StalledIssuesModel::runMessageBox(failedInfo);
}

void NameConflictedStalledIssue::showLocalRenameHasFailedMessageBox(bool isFile)
{
    QMegaMessageBox::MessageBoxInfo failedInfo;
    failedInfo.title = MegaSyncApp->getMEGAString();
    failedInfo.textFormat = Qt::RichText;
    failedInfo.buttons = QMessageBox::Yes;
    if(isFile)
    {
        failedInfo.text = StalledIssuesStrings::RemoveFileFailedTitle();
        failedInfo.informativeText = StalledIssuesStrings::RemoveLocalFileFailedDescription();
    }
    else
    {
        failedInfo.text = StalledIssuesStrings::RemoveFolderFailedTitle();
        failedInfo.informativeText = StalledIssuesStrings::RemoveLocalFolderFailedDescription();
    }

    StalledIssuesModel::runMessageBox(failedInfo);
}

bool NameConflictedStalledIssue::hasDuplicatedNodes() const
{
    for(int index = 0; index < mCloudConflictedNames.size(); ++index)
    {
        if(mCloudConflictedNames.hasDuplicatedNodes(index))
        {
            return true;
        }
    }

    return false;
}

bool NameConflictedStalledIssue::areAllDuplicatedNodes() const
{
    for(int index = 0; index < mCloudConflictedNames.size(); ++index)
    {
        if(!mCloudConflictedNames.areAllDuplicatedNodes(index))
        {
            return false;
        }
    }

    return mCloudConflictedNames.size() > 1;
}

const QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>>& NameConflictedStalledIssue::getNameConflictLocalData() const
{
    return mLocalConflictedNames;
}

const NameConflictedStalledIssue::CloudConflictedNames& NameConflictedStalledIssue::getNameConflictCloudData() const
{
    return mCloudConflictedNames;
}

void NameConflictedStalledIssue::setLocalFailed(int errorConflictIndex, const QString& error)
{
    auto conflictedNames(mLocalConflictedNames);
    for(int index = 0; index < conflictedNames.size(); ++index)
    {
        index == errorConflictIndex ? conflictedNames.at(index)->setFailed(error)
                                    : conflictedNames.at(index)->setFailed();
    }
}

void NameConflictedStalledIssue::setCloudFailed(int errorConflictIndex, const QString& error)
{
    auto conflictedNames(mCloudConflictedNames.getConflictedNames());
    for(int index = 0; index < conflictedNames.size(); ++index)
    {
        index == errorConflictIndex ? conflictedNames.at(index)->setFailed(error)
                                    : conflictedNames.at(index)->setFailed();
    }
}

bool NameConflictedStalledIssue::containsHandle(mega::MegaHandle handle)
{
    foreach(auto& cloudConflictedName, mCloudConflictedNames.getConflictedNames())
    {
        if(cloudConflictedName->mHandle == handle)
        {
            mLastModifiedNode.cloudItem = cloudConflictedName;
            if(getCloudData()->mPathHandle == handle)
            {
                mLastModifiedNode.usedAsCloudDataHandle = true;
            }
            return true;
        }
    }

    return false;
}

void NameConflictedStalledIssue::updateHandle(mega::MegaHandle handle)
{
    if(mLastModifiedNode.isValid())
    {
        std::unique_ptr<mega::MegaNode> oldNode(MegaSyncApp->getMegaApi()->getNodeByHandle(mLastModifiedNode.cloudItem->mHandle));
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
        if(node && oldNode)
        {
            mLastModifiedNode.cloudItem->mHandle = handle;

            if(node->isFile())
            {
                mCloudConflictedNames.updateFileConflictedName(node->getModificationTime(), node->getSize(), oldNode->getCreationTime(), node->getCreationTime(), QString::fromUtf8(node->getFingerprint()), mLastModifiedNode.cloudItem);
            }
        }

        mLastModifiedNode.cloudItem->mHandle = handle;
        if(auto remoteAttr = std::dynamic_pointer_cast<RemoteFileFolderAttributes>(mLastModifiedNode.cloudItem->mItemAttributes))
        {
            remoteAttr->setHandle(handle);
        }
        if(mLastModifiedNode.usedAsCloudDataHandle)
        {
            getCloudData()->setPathHandle(handle);
        }
    }
}

void NameConflictedStalledIssue::updateName()
{
    if(mLastModifiedNode.isValid() && !mLastModifiedNode.cloudItem->isSolved())
    {
        std::unique_ptr<mega::MegaNode> newNode(MegaSyncApp->getMegaApi()->getNodeByHandle(mLastModifiedNode.cloudItem->mHandle));
        if(newNode)
        {
            QString newName(QString::fromUtf8(newNode->getName()));
            mLastModifiedNode.cloudItem->solveByRename(newName);

            checkAndSolveConflictedNamesSolved();
        }
    }
}

bool NameConflictedStalledIssue::checkForExternalChanges()
{
    if(!mLocalConflictedNames.isEmpty())
    {
        foreach(auto conflictedName, mLocalConflictedNames)
        {
            if(!conflictedName->isSolved())
            {
                conflictedName->checkExternalChange();

                if(conflictedName->isSolved())
                {
                    if(checkAndSolveConflictedNamesSolved())
                    {
                        setIsSolved(SolveType::POTENTIALLY_SOLVED);
                        break;
                    }
                }
            }
        }
    }

    if(!isPotentiallySolved())
    {
        auto conflictedNames(mCloudConflictedNames.getConflictedNames());
        if(!conflictedNames.isEmpty())
        {
            foreach(auto conflictedName, conflictedNames)
            {
                if(!conflictedName->isSolved())
                {
                    conflictedName->checkExternalChange();

                    if(conflictedName->isSolved())
                    {
                        if(checkAndSolveConflictedNamesSolved())
                        {
                            setIsSolved(SolveType::POTENTIALLY_SOLVED);
                            break;
                        }
                    }
                }
            }
        }
    }

    return isPotentiallySolved();
}

bool NameConflictedStalledIssue::hasFoldersToMerge() const
{
    uint8_t cloudFolders(0);
    auto conflictedNames(mCloudConflictedNames.getConflictedNames());

    foreach(const auto& cloudConflictedName, conflictedNames)
    {
        std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudConflictedName->mHandle));
        if(conflictedNode && conflictedNode->isFolder())
        {
            cloudFolders++;
            if(cloudFolders > 1)
            {
                break;
            }
        }
    }

    return cloudFolders > 1;
}

bool NameConflictedStalledIssue::solveLocalConflictedNameByRemove(int conflictIndex)
{
    auto result(false);
    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];

        conflictName->solveByRemove();
        result = checkAndSolveConflictedNamesSolved();
    }

    return result;
}

bool NameConflictedStalledIssue::solveCloudConflictedNameByRemove(int conflictIndex)
{
    auto result(false);

    auto conflictedNames(mCloudConflictedNames.getConflictedNames());
    if(conflictedNames.size() > conflictIndex)
    {
        auto conflictName = conflictedNames.at(conflictIndex);

        if(conflictName)
        {
            conflictName->solveByRemove();
            result = checkAndSolveConflictedNamesSolved();
        }
    }

    return result;
}

bool NameConflictedStalledIssue::solveLocalConflictedNameByRename(int conflictIndex, const QString &renameTo, const QString& renameFrom)
{
    auto result(false);

    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];

        auto cloudConflictedNames(mCloudConflictedNames.getConflictedNames());
        auto siblingItem(findOtherSideItem(cloudConflictedNames, conflictName));
        if(siblingItem)
        {
            result = renameCloudSibling(siblingItem, renameTo);
        }

        //Undo the local name change
        if(!result)
        {
            QFileInfo originalFileInfo(conflictName->mConflictedPath);
            QFileInfo newFileInfo(originalFileInfo.path(), renameTo);
            QFile file(newFileInfo.filePath());
            if(file.exists())
            {
                file.rename(QDir::toNativeSeparators(originalFileInfo.filePath()));
            }

            //Fail string pending
            conflictName->setFailed(tr("Unable to rename the file in MEGA"));
        }

        if(result)
        {
            conflictName->solveByRename(renameTo);
        }

        if(!siblingItem || result)
        {
            result = checkAndSolveConflictedNamesSolved();
        }
    }

    return result;
}

bool NameConflictedStalledIssue::solveCloudConflictedNameByRename(int conflictIndex, const QString& renameTo, const QString& renameFrom)
{
    auto result(false);

    auto conflictedNames(mCloudConflictedNames.getConflictedNames());
    if(conflictedNames.size() > conflictIndex)
    {
        auto conflictName = conflictedNames.at(conflictIndex);
        if(conflictName)
        {

            auto siblingItem(findOtherSideItem(mLocalConflictedNames, conflictName));
            if(siblingItem)
            {
                result = renameLocalSibling(siblingItem, renameTo);
            }

            if(!result)
            {
                std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictName->mHandle));
                if(conflictedNode)
                {
                    MegaApiSynchronizedRequest::runRequest(
                        &mega::MegaApi::renameNode,
                        MegaSyncApp->getMegaApi(),
                        conflictedNode.get(),
                        renameFrom.toStdString().c_str());

                    //Fail string pending
                    conflictName->setFailed(tr("Unable to rename the local file"));
                }
            }

            if(result)
            {
                conflictName->solveByRename(renameTo);
            }

            if(!siblingItem || result)
            {
                result = checkAndSolveConflictedNamesSolved();
            }
        }
    }

    return result;
}

bool NameConflictedStalledIssue::renameNodesAutomatically()
{
    auto sortLogic = [](QList<std::shared_ptr<ConflictedNameInfo>>& names){
        std::sort(names.begin(), names.end(), [](const std::shared_ptr<ConflictedNameInfo>& check1, const std::shared_ptr<ConflictedNameInfo>& check2){
            return check1->mItemAttributes->modifiedTime() > check2->mItemAttributes->modifiedTime();
        });
    };

    auto result(true);

    auto cloudConflictedNames(mCloudConflictedNames.getConflictedNames());
    sortLogic(cloudConflictedNames);
    auto localConflictedNames(mLocalConflictedNames);
    sortLogic(localConflictedNames);

    QStringList itemsBeingRenamed;

    if(localConflictedNames.isEmpty())
    {
        result = renameCloudNodesAutomatically(cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed);
    }
    else if(cloudConflictedNames.isEmpty())
    {
        result = renameLocalItemsAutomatically(cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed);
    }
    else
    {
        auto lastModifiedCloudName = cloudConflictedNames.first();
        auto lastModifiedLocalName = localConflictedNames.first();

        if(lastModifiedCloudName->mItemAttributes->modifiedTime() >
            lastModifiedLocalName->mItemAttributes->modifiedTime())
        {
            if((result = renameCloudNodesAutomatically(
                   cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed)))
            {
                result = renameLocalItemsAutomatically(
                    cloudConflictedNames, localConflictedNames, false, itemsBeingRenamed);
            }
        }
        else
        {
            if((result = renameLocalItemsAutomatically(
                    cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed)))
            {
                result = renameCloudNodesAutomatically(
                    cloudConflictedNames, localConflictedNames, false, itemsBeingRenamed);
            }
        }
    }

    return result;
}

bool NameConflictedStalledIssue::renameCloudNodesAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                                               const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                                               bool ignoreLastModifiedName,
                                                               QStringList& cloudItemsBeingRenamed)
{
    auto result(true);
    for(int index = cloudConflictedNames.size() - 1; index >= 0; --index)
    {
        auto& cloudConflictedName = cloudConflictedNames.at(index);

        if(!cloudConflictedName->isSolved())
        {
            auto localConflictedName = findOtherSideItem(localConflictedNames, cloudConflictedName);

            if(index == 0 && ignoreLastModifiedName)
            {
                cloudConflictedName->solveByOtherSide();
                if(localConflictedName)
                {
                    localConflictedName->solveByOtherSide();
                }
            }
            else
            {

                std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudConflictedName->mHandle));
                if(conflictedNode)
                {
                    std::shared_ptr<mega::MegaError> error(nullptr);

                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedNode->getParentHandle()));
                    auto newName = Utilities::getNonDuplicatedNodeName(conflictedNode.get(), parentNode.get(), cloudConflictedName->getConflictedName(), true, cloudItemsBeingRenamed);
                    MegaApiSynchronizedRequest::runRequestWithResult(
                        &mega::MegaApi::renameNode,
                        MegaSyncApp->getMegaApi(),
                        [this, &error](const mega::MegaRequest&, const mega::MegaError& e)
                        {
                            if(e.getErrorCode() != mega::MegaError::API_OK)
                            {
                                error.reset(e.copy());
                            }
                        },
                        conflictedNode.get(),
                        newName.toStdString().c_str());

                    if(error)
                    {
                        result = false;
                        cloudConflictedName->setFailed(RenameRemoteNodeDialog::renamedFailedErrorString(error.get(), conflictedNode->isFile()));
                        break;
                    }
                    else
                    {
                        cloudItemsBeingRenamed.append(newName);
                        renameLocalSibling(localConflictedName, newName);
                        cloudConflictedName->solveByRename(newName);
                    }
                }
            }
        }
    }

    return result;
}

bool NameConflictedStalledIssue::renameLocalItemsAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                                               const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                                               bool ignoreLastModifiedName,
                                                               QStringList& cloudItemsBeingRenamed)
{
    auto result(true);
    for(int index = localConflictedNames.size() - 1; index >= 0; --index)
    {
        auto& localConflictedName = localConflictedNames.at(index);

        if(!localConflictedName->isSolved())
        {
            auto cloudConflictedName = findOtherSideItem(cloudConflictedNames, localConflictedName);

            if(index == 0 && ignoreLastModifiedName)
            {
                localConflictedName->solveByOtherSide();

                if(cloudConflictedName)
                {
                    cloudConflictedName->solveByOtherSide();
                }
            }
            else
            {
                QFileInfo fileInfo(localConflictedName->mConflictedPath);
                fileInfo.setFile(fileInfo.path(), localConflictedName->getConflictedName());

                QFile file(fileInfo.filePath());
                if(file.exists())
                {
                    auto newName = Utilities::getNonDuplicatedLocalName(fileInfo, true, cloudItemsBeingRenamed);

                    fileInfo.setFile(fileInfo.path(), newName);
                    if(file.rename(QDir::toNativeSeparators(fileInfo.filePath())))
                    {
                        localConflictedName->solveByRename(newName);
                        renameCloudSibling(cloudConflictedName, newName);
                    }
                    else
                    {
                        result = false;
                        localConflictedName->setFailed(RenameLocalNodeDialog::renamedFailedErrorString(fileInfo.isFile()));
                        break;
                    }
                }
            }
        }
    }

    return result;
}

bool NameConflictedStalledIssue::renameCloudSibling(std::shared_ptr<ConflictedNameInfo> item, const QString &newName)
{
    std::shared_ptr<mega::MegaError> error(nullptr);
    if(item)
    {
        std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle(item->mHandle));
        if(conflictedNode)
        {
            std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedNode->getParentHandle()));
            MegaApiSynchronizedRequest::runRequestWithResult(
                &mega::MegaApi::renameNode,
                MegaSyncApp->getMegaApi(),
                [&error](const mega::MegaRequest&, const mega::MegaError& e)
                {
                    if(e.getErrorCode() != mega::MegaError::API_OK)
                    {
                        error.reset(e.copy());
                    }
                },
                conflictedNode.get(),
                newName.toStdString().c_str());

            error ? item->setFailed(RenameRemoteNodeDialog::renamedFailedErrorString(error.get(), conflictedNode->isFile())) : item->solveByRename(newName);
        }
    }

    return error == nullptr;
}

bool NameConflictedStalledIssue::renameLocalSibling(std::shared_ptr<ConflictedNameInfo> item, const QString &newName)
{
    auto result(false);
    if(item)
    {
        QFileInfo fileInfo(item->mConflictedPath);
        fileInfo.setFile(fileInfo.path(), item->getConflictedName());

        QFile file(fileInfo.filePath());
        if(file.exists())
        {
            fileInfo.setFile(fileInfo.path(), newName);
            result = file.rename(QDir::toNativeSeparators(fileInfo.filePath()));
            result ?  item->solveByRename(newName) : item->setFailed(RenameLocalNodeDialog::renamedFailedErrorString(fileInfo.isFile()));
        }
    }

    return result;
}

std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> NameConflictedStalledIssue::findOtherSideItem(const QList<std::shared_ptr<ConflictedNameInfo>>& items, std::shared_ptr<ConflictedNameInfo> check)
{
    std::shared_ptr<ConflictedNameInfo> partialFound(nullptr);

    auto it = std::find_if(items.begin(), items.end(), [&partialFound, check](const std::shared_ptr<ConflictedNameInfo>& fileIt){
        if(!fileIt->isSolved())
        {
            auto sameFingerprint(false);
            auto sameName(check->getConflictedName().compare(fileIt->getConflictedName(), Qt::CaseSensitive) == 0);
            if(check->mItemAttributes->size() != 0 && (check->mItemAttributes->size() == check->mItemAttributes->size()))
            {
                auto fp1(check->mItemAttributes->fingerprint());
                auto fp2(fileIt->mItemAttributes->fingerprint());
                if(!fp1.isEmpty() && !fp2.isEmpty())
                {
                    sameFingerprint = (fp1.compare(fp2) == 0);
                }
            }

            if(sameFingerprint)
            {
                return true;
            }
            else if(sameName)
            {
                partialFound = fileIt;
            }
            return false;
        }
        else
        {
            return false;
        }
    });

    if(it != items.end())
    {
        return (*it);
    }
    else if(partialFound)
    {
        return partialFound;
    }
    else
    {
        return nullptr;
    }
}

bool NameConflictedStalledIssue::checkAndSolveConflictedNamesSolved()
{
    auto checkLogic = [](const QList<std::shared_ptr<ConflictedNameInfo>>& conflicts) -> int
    {
        auto unsolvedItems(0);
        for (auto it = conflicts.begin(); it != conflicts.end(); ++it)
        {
            if(!(*it)->isSolved())
            {
                unsolvedItems++;
            }
        }

        if(unsolvedItems < 2)
        {
            for (auto it = conflicts.begin(); it != conflicts.end(); ++it)
            {
                if(!(*it)->isSolved())
                {
                    (*it)->solveByOtherSide();
                    unsolvedItems--;
                }
            }
        }

        return unsolvedItems;
    };

    auto unsolvedItems(0);
    auto cloudConflicts = mCloudConflictedNames.getConflictedNames();
    if(cloudConflicts.size() > mLocalConflictedNames.size())
    {
        unsolvedItems = checkLogic(cloudConflicts);
        if(unsolvedItems == 0 && !mLocalConflictedNames.isEmpty())
        {
            unsolvedItems = checkLogic(mLocalConflictedNames);
        }
    }
    else
    {
        unsolvedItems = checkLogic(mLocalConflictedNames);
        if(unsolvedItems == 0 && !cloudConflicts.isEmpty())
        {
            unsolvedItems = checkLogic(cloudConflicts);
        }
    }

    return unsolvedItems == 0;
}

bool NameConflictedStalledIssue::semiAutoSolveIssue(ActionsSelected option)
{
    return solveIssue(option);
}

//This code is never called. NameConflict, for the moment, are not autosolvable.
bool NameConflictedStalledIssue::autoSolveIssue()
{
    setAutoResolutionApplied(true);
    ActionsSelected options(ActionSelected::RemoveDuplicated | ActionSelected::Rename | ActionSelected::MergeFolders);
    auto result = solveIssue(options);
    if(result)
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_AUTOMATICALLY);
        return true;
    }

    return false;
}

bool NameConflictedStalledIssue::isAutoSolvable() const
{
    return Preferences::instance()->isStalledIssueSmartModeActivated();
}

bool NameConflictedStalledIssue::solveIssue(ActionsSelected option)
{
    auto result(false);

    if(option & ActionSelected::MergeFolders && foldersCount() > 1)
    {
        auto errorInfo = mCloudConflictedNames.mergeFolders();
        if(!errorInfo.error.isEmpty())
        {
            setCloudFailed(errorInfo.conflictIndex, errorInfo.error);
            result= false;
        }

        if(result)
        {
            result = checkAndSolveConflictedNamesSolved();
        }
    }

    if(!result && option & ActionSelected::RemoveDuplicated)
    {
        result = mCloudConflictedNames.removeDuplicatedNodes() == nullptr;
        if(result)
        {
            result = checkAndSolveConflictedNamesSolved();
        }
    }

    if(!result && option & ActionSelected::KeepMostRecentlyModifiedNode)
    {
        result = mCloudConflictedNames.keepMostRecentlyModifiedNode() == nullptr;
        if(result)
        {
            result = checkAndSolveConflictedNamesSolved();
        }
    }

    if(!result && option & ActionSelected::Rename)
    {
        result = renameNodesAutomatically();
        if(result)
        {
            checkAndSolveConflictedNamesSolved();
        }
    }

    return result;
}

//CloudConflictedNames logic
std::shared_ptr<mega::MegaError>
NameConflictedStalledIssue::CloudConflictedNames::removeDuplicatedNodes()
{
    for(int index = 0; index < mConflictedNames.size(); ++index)
    {
        auto& conflictedNamesGroup = mConflictedNames[index];

        if(conflictedNamesGroup.conflictedNames.size() > 1)
        {
            //The object is auto deleted when finished (as it needs to survive this issue)
            foreach(auto conflictedName, conflictedNamesGroup.conflictedNames)
            {
                if(conflictedName->getSolvedType() ==
                        NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED &&
                    conflictedName != (*(conflictedNamesGroup.conflictedNames.end() - 1)))
                {
                    auto moveToBinErrors = MoveToMEGABin::moveToBin(
                        conflictedName->mHandle, QLatin1String("SyncDuplicated"), true);
                    if(!moveToBinErrors.binFolderCreationError && !moveToBinErrors.moveError)
                    {
                        conflictedName->solveByRemove();
                    }
                    else
                    {
                        std::shared_ptr<mega::MegaError> error;
                        if(moveToBinErrors.binFolderCreationError)
                        {
                            error = moveToBinErrors.binFolderCreationError;
                        }
                        else
                        {
                            error = moveToBinErrors.moveError;
                        }

                        auto errorStr = StalledIssuesStrings::RemoveRemoteFailedFile(error.get());
                        conflictedName->setFailed(errorStr);

                        return error;
                    }
                }
            }

            conflictedNamesGroup.solved = true;
        }
    }

    mDuplicatedSolved = true;

    //No error to return
    return nullptr;
}

std::shared_ptr<mega::MegaError>
NameConflictedStalledIssue::CloudConflictedNames::keepMostRecentlyModifiedNode()
{
    auto info = findMostRecentlyModifiedNode();
    foreach(auto conflictedName, info.oldVersions)
    {
        std::unique_ptr<mega::MegaNode> conflictNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedName->mHandle));
        if(conflictNode)
        {
            auto error = StalledIssuesUtilities::removeRemoteFile(
                conflictNode.get());
            if(error)
            {
                auto errorStr = StalledIssuesStrings::RemoveRemoteFailedFile(
                    error.get());
                conflictedName->setFailed(errorStr);

                return error;
            }
            else
            {
                conflictedName->solveByRemove();
            }
        }
    }

    //No error to return
    return nullptr;
}

NameConflictedStalledIssue::CloudConflictedNames::MostRecentlyModifiedInfo
NameConflictedStalledIssue::CloudConflictedNames::findMostRecentlyModifiedNode() const
{
    MostRecentlyModifiedInfo info;

    for(int index = 0; index < mConflictedNames.size(); ++index)
    {
        const auto conflictedNamesGroup = mConflictedNames[index];
        foreach(auto conflictedName, conflictedNamesGroup.conflictedNames)
        {
            if(conflictedName->getSolvedType() ==
                NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
            {
                if(info.mostRecentlyModified == nullptr)
                {
                    info.mostRecentlyModified = conflictedName;
                }
                else
                {
                    QDateTime previousConflictDate;
                    QDateTime currentConflictDate;

                    info.mostRecentlyModified->mItemAttributes->requestModifiedTime(MegaSyncApp,
                        [&previousConflictDate](const QDateTime& time)
                        { previousConflictDate = time; });
                    conflictedName->mItemAttributes->requestModifiedTime(MegaSyncApp,
                        [&currentConflictDate](const QDateTime& time)
                        { currentConflictDate = time; });

                    if(currentConflictDate >= previousConflictDate)
                    {
                        info.oldVersions.append(info.mostRecentlyModified);
                        info.mostRecentlyModified = conflictedName;
                    }
                    else
                    {
                        info.oldVersions.append(conflictedName);
                    }
                }
            }
        }
    }

    return info;
}

NameConflictedStalledIssue::CloudConflictedNames::MergeFoldersError
NameConflictedStalledIssue::CloudConflictedNames::mergeFolders()
{
    MergeFoldersError errorInfo;

    auto conflictedNames = getConflictedNames();
    if(!conflictedNames.isEmpty())
    {
        std::sort(conflictedNames.begin(), conflictedNames.end(),
            [](std::shared_ptr<ConflictedNameInfo> info1, std::shared_ptr<ConflictedNameInfo> info2)
            {
                auto info1FileCount(-1);
                if(!info1->mIsFile)
                {
                    auto file1Attr = std::dynamic_pointer_cast<RemoteFileFolderAttributes>(info1->mItemAttributes);
                    info1FileCount = file1Attr->fileCount();
                }

                auto info2FileCount(-1);
                if(!info2->mIsFile)
                {
                    auto fileAttr = std::dynamic_pointer_cast<RemoteFileFolderAttributes>(info2->mItemAttributes);
                    info2FileCount = fileAttr->fileCount();
                }

                return info1FileCount > info2FileCount;
            });

        auto biggestFolder(conflictedNames.takeFirst());
        std::unique_ptr<mega::MegaNode> targetFolder(MegaSyncApp->getMegaApi()->getNodeByHandle(biggestFolder->mHandle));
        for(int index = 0; index < conflictedNames.size(); ++index)
        {
            auto conflictedFolder(conflictedNames.at(index));
            std::unique_ptr<mega::MegaNode> folderToMerge(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedFolder->mHandle));
            if(folderToMerge && folderToMerge->isFolder())
            {
                MergeMEGAFolders mergeItem(targetFolder.get(), folderToMerge.get());
                auto error = mergeItem.merge(MergeMEGAFolders::ActionForDuplicates::IgnoreAndMoveToBin);
                if(error)
                {
                    errorInfo.conflictIndex = index;
                    errorInfo.error = tr("Unable to merge this folder.");
                    break;
                }
                else
                {
                    conflictedFolder->solveByMerge();
                }
            }
        }
    }

    return errorInfo;
}
