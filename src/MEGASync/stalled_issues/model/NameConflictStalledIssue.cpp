#include "NameConflictStalledIssue.h"

#include "mega/types.h"
#include "Utilities.h"
#include "AppStatsEvents.h"

//Name conflict Stalled Issue
NameConflictedStalledIssue::NameConflictedStalledIssue(const NameConflictedStalledIssue &tdr)
    : StalledIssue(tdr)
{
    qRegisterMetaType<NameConflictedStalledIssue>("NameConflictedStalledIssue");
}

NameConflictedStalledIssue::NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue)
{
}

void NameConflictedStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    mReason = stall->reason();
    auto localConflictNames = stall->pathCount(false);

    if(localConflictNames > 0)
    {
        initLocalIssue(stall);

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
        initCloudIssue(stall);

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
            auto cloudPath = QString::fromUtf8(stall->path(true,index));
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

const NameConflictedStalledIssue::CloudConflictedNamesByHandle& NameConflictedStalledIssue::getNameConflictCloudData() const
{
    return mCloudConflictedNames;
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
                    checkAndSolveConflictedNamesSolved(true);
                }
            }

            if(isPotentiallySolved())
            {
                break;
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
                        checkAndSolveConflictedNamesSolved(true);
                    }
                }

                if(isPotentiallySolved())
                {
                    break;
                }
            }
        }
    }

    return isPotentiallySolved();
}

bool NameConflictedStalledIssue::solveLocalConflictedNameByRemove(int conflictIndex)
{
    auto result(false);
    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];
        conflictName->mSolved = ConflictedNameInfo::SolvedType::REMOVE;

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
            conflictName->mSolved = ConflictedNameInfo::SolvedType::REMOVE;

            result = checkAndSolveConflictedNamesSolved();
        }
    }

    return result;
}

bool NameConflictedStalledIssue::solveLocalConflictedNameByRename(int conflictIndex, const QString &renameTo)
{
    auto result(false);

    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];
        conflictName->solveByRename(renameTo);

        auto cloudConflictedNames(mCloudConflictedNames.getConflictedNames());
        renameCloudSibling(findOtherSideItem(cloudConflictedNames, conflictName), renameTo);

        result = checkAndSolveConflictedNamesSolved();
    }

    return result;
}

bool NameConflictedStalledIssue::solveCloudConflictedNameByRename(int conflictIndex, const QString &renameTo)
{
    auto result(false);

    auto conflictedNames(mCloudConflictedNames.getConflictedNames());
    if(conflictedNames.size() > conflictIndex)
    {
        auto conflictName = conflictedNames.at(conflictIndex);
        if(conflictName)
        {
            conflictName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
            conflictName->solveByRename(renameTo);

            renameLocalSibling(findOtherSideItem(mLocalConflictedNames, conflictName), renameTo);

            result = checkAndSolveConflictedNamesSolved();
        }
    }

    return result;
}


void NameConflictedStalledIssue::renameNodesAutomatically()
{
    auto sortLogic = [](QList<std::shared_ptr<ConflictedNameInfo>>& names){
        std::sort(names.begin(), names.end(), [](const std::shared_ptr<ConflictedNameInfo>& check1, const std::shared_ptr<ConflictedNameInfo>& check2){
            return check1->mItemAttributes->modifiedTime() > check2->mItemAttributes->modifiedTime();
        });
    };

    auto cloudConflictedNames(mCloudConflictedNames.getConflictedNames());
    sortLogic(cloudConflictedNames);
    auto localConflictedNames(mLocalConflictedNames);
    sortLogic(localConflictedNames);

    QStringList itemsBeingRenamed;

    if(localConflictedNames.isEmpty())
    {
        renameCloudNodesAutomatically(cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed);
    }
    else if(cloudConflictedNames.isEmpty())
    {
        renameLocalItemsAutomatically(cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed);
    }
    else
    {
        auto lastModifiedCloudName = cloudConflictedNames.first();
        auto lastModifiedLocalName = localConflictedNames.first();

        if(lastModifiedCloudName->mItemAttributes->modifiedTime() > lastModifiedLocalName->mItemAttributes->modifiedTime())
        {
            renameCloudNodesAutomatically(cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed);
            renameLocalItemsAutomatically(cloudConflictedNames, localConflictedNames, false, itemsBeingRenamed);
        }
        else
        {
            renameLocalItemsAutomatically(cloudConflictedNames, localConflictedNames, true, itemsBeingRenamed);
            renameCloudNodesAutomatically(cloudConflictedNames, localConflictedNames, false, itemsBeingRenamed);
        }
    }
}

void NameConflictedStalledIssue::renameCloudNodesAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                                               const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                                               bool ignoreLastModifiedName,
                                                               QStringList& cloudItemsBeingRenamed)
{
    auto counter(0);
    foreach(auto& cloudConflictedName, cloudConflictedNames)
    {
        if(cloudConflictedName->mSolved
                == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
        {
            auto localConflictedName = findOtherSideItem(localConflictedNames, cloudConflictedName);

            if(counter == 0 && ignoreLastModifiedName)
            {
                cloudConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
                if(localConflictedName)
                {
                    localConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
                }
            }
            else
            {

                std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudConflictedName->mHandle));
                if(conflictedNode)
                {
                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedNode->getParentHandle()));
                    auto newName = Utilities::getNonDuplicatedNodeName(conflictedNode.get(), parentNode.get(), cloudConflictedName->getConflictedName(), true, cloudItemsBeingRenamed);
                    MegaSyncApp->getMegaApi()->renameNode(conflictedNode.get(), newName.toUtf8().constData());

                    cloudItemsBeingRenamed.append(newName);

                    renameLocalSibling(localConflictedName, newName);

                    cloudConflictedName->solveByRename(newName);
                }
            }

            counter++;
        }
    }
}

void NameConflictedStalledIssue::renameLocalItemsAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                                               const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                                               bool ignoreLastModifiedName,
                                                               QStringList& cloudItemsBeingRenamed)
{
    auto counter(0);

    foreach(auto& localConflictedName, localConflictedNames)
    {
        if(localConflictedName->mSolved
           == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
        {
            auto cloudConflictedName = findOtherSideItem(cloudConflictedNames, localConflictedName);

            if(counter == 0 && ignoreLastModifiedName)
            {
                localConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;

                if(cloudConflictedName)
                {
                    cloudConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
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
                    }

                    renameCloudSibling(cloudConflictedName, newName);
                }
            }
        }
        counter++;
    }
}

void NameConflictedStalledIssue::renameCloudSibling(std::shared_ptr<ConflictedNameInfo> item, const QString &newName)
{
    if(item)
    {
        std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle(item->mHandle));
        if(conflictedNode)
        {
            std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedNode->getParentHandle()));
            MegaSyncApp->getMegaApi()->renameNode(conflictedNode.get(), newName.toUtf8().constData());

            item->solveByRename(newName);
        }
    }
}

void NameConflictedStalledIssue::renameLocalSibling(std::shared_ptr<ConflictedNameInfo> item, const QString &newName)
{
    if(item)
    {
        QFileInfo fileInfo(item->mConflictedPath);
        fileInfo.setFile(fileInfo.path(), item->getConflictedName());

        QFile file(fileInfo.filePath());
        if(file.exists())
        {
            fileInfo.setFile(fileInfo.path(), newName);
            if(file.rename(QDir::toNativeSeparators(fileInfo.filePath())))
            {
                item->solveByRename(newName);
            }
        }
    }
}

std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> NameConflictedStalledIssue::findOtherSideItem(const QList<std::shared_ptr<ConflictedNameInfo>>& items, std::shared_ptr<ConflictedNameInfo> check)
{
    auto it = std::find_if(items.begin(), items.end(), [check](const std::shared_ptr<ConflictedNameInfo>& fileIt){
        if(fileIt->mSolved
           == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
        {
            auto fp1(check->mItemAttributes->fingerprint());
            auto fp2(fileIt->mItemAttributes->fingerprint());
            if(!fp1.isEmpty() && !fp2.isEmpty())
            {
                return fp1.compare(fp2) == 0;
            }
            else
            {
                return check->getUnescapeConflictedName().compare(fileIt->getUnescapeConflictedName(), Qt::CaseSensitive) == 0;
            }
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
    else
    {
        return nullptr;
    }
}

bool NameConflictedStalledIssue::checkAndSolveConflictedNamesSolved(bool isPotentiallySolved)
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
                    (*it)->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
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

    if(!isSolved() && unsolvedItems == 0)
    {
        setIsSolved(isPotentiallySolved);
    }

    return mIsSolved;
}

void NameConflictedStalledIssue::semiAutoSolveIssue(int option)
{
    solveIssue(option);
}

void NameConflictedStalledIssue::autoSolveIssue()
{
    solveIssue(ActionSelected::RemoveDuplicated | ActionSelected::Rename | ActionSelected::MergeFolders);
    if(isSolved())
    {
        MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_AUTOMATICALLY, "Name conflict issue solved automatically", false, nullptr);
    }
}

void NameConflictedStalledIssue::solveIssue(int option)
{
    auto result(false);

    if(option & ActionSelected::MergeFolders && hasFolders() > 1)
    {
        mCloudConflictedNames.mergeFolders();
        result = checkAndSolveConflictedNamesSolved();
    }

    if(!result & option & ActionSelected::RemoveDuplicated)
    {
        mCloudConflictedNames.removeDuplicatedNodes();
        result = checkAndSolveConflictedNamesSolved();
    }

    if(!result && option & ActionSelected::Rename)
    {
        renameNodesAutomatically();
        checkAndSolveConflictedNamesSolved();
    }
}
