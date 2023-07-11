#include "NameConflictStalledIssue.h"

#include "mega/types.h"

//Name conflict Stalled Issue
NameConflictedStalledIssue::NameConflictedStalledIssue(const NameConflictedStalledIssue &tdr)
    : StalledIssue(tdr)
{
    qRegisterMetaType<NameConflictedStalledIssue>("NameConflictedStalledIssue");
}

NameConflictedStalledIssue::NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue()
{
    mReason = stallIssue->reason();
    fillIssue(stallIssue);
    endFillingIssue();
}

void NameConflictedStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
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
            if(localPath.isFile())
            {
                info->mItemAttributes->requestModifiedTime(nullptr, nullptr);
            }
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

            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudHandle));
            if(node)
            {
                QFileInfo cloudPathInfo(cloudPath);
                std::shared_ptr<ConflictedNameInfo> info(new ConflictedNameInfo(cloudPathInfo, node->isFile(), std::make_shared<RemoteFileFolderAttributes>(node->getHandle(), nullptr)));
                info->mHandle = cloudHandle;

                if(node->isFile())
                {
                    //Use for autosolve
                    info->mItemAttributes->requestModifiedTime(nullptr, nullptr);

                    mCloudConflictedNames.addFileConflictedName(node->getModificationTime(), node->getSize(), node->getCreationTime(), QString::fromUtf8(node->getFingerprint()), info);
                    mFiles++;
                }
                else
                {
                    mCloudConflictedNames.addFolderConflictedName(cloudHandle, info);
                    mFolders++;
                }
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

bool NameConflictedStalledIssue::hasDuplicatedNodes() const
{
    for(int index = 0; index < mCloudConflictedNames.size(); ++index)
    {
        if(mCloudConflictedNames.hasDuplicatedNodes())
        {
            return true;
        }
    }

    return false;
}

const QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>>& NameConflictedStalledIssue::getNameConflictLocalData() const
{
    return mLocalConflictedNames;
}

const NameConflictedStalledIssue::CloudConflictedNamesByHandle& NameConflictedStalledIssue::getNameConflictCloudData() const
{
    return mCloudConflictedNames;
}

bool NameConflictedStalledIssue::solveLocalConflictedNameByRemove(int conflictIndex)
{
    auto result(false);
    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];
        conflictName->mSolved = ConflictedNameInfo::SolvedType::REMOVE;

        result = checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mCloudConflictedNames.getConflictedNames());
        }
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

            result = checkAndSolveConflictedNamesSolved(conflictedNames);
            if(result)
            {
                checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
            }
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
        conflictName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
        conflictName->mRenameTo = renameTo;

        result = checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mCloudConflictedNames.getConflictedNames());
        }
    }

    return result;
}

void NameConflictedStalledIssue::renameNodesAutomatically()
{
    auto sortLogic = [](QList<std::shared_ptr<ConflictedNameInfo>>& names){
        std::sort(names.begin(), names.end(), [](const std::shared_ptr<ConflictedNameInfo>& check1, const std::shared_ptr<ConflictedNameInfo>& check2){
            if(check1->mIsFile && check2->mIsFile)
            {
                return check1->mItemAttributes->modifiedTime() > check2->mItemAttributes->modifiedTime();
            }

            return check1->mIsFile;
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

void NameConflictedStalledIssue::renameCloudNodesAutomatically(const QList<std::shared_ptr<ConflictedNameInfo> >& cloudConflictedNames,
                                                               const QList<std::shared_ptr<ConflictedNameInfo> > &localConflictedNames,
                                                               bool ignoreLastModifiedName,
                                                               QStringList& cloudItemsBeingRenamed)
{
    auto counter(0);
    foreach(auto& cloudConflictedName, cloudConflictedNames)
    {
        if(cloudConflictedName->mSolved
                == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
        {
            auto localConflictedName = std::find_if(localConflictedNames.begin(), localConflictedNames.end(), [cloudConflictedName](const std::shared_ptr<ConflictedNameInfo>& check){

                if(check->mSolved
                   == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
                {
                    auto fp1(check->mItemAttributes->fingerprint());
                    auto fp2(cloudConflictedName->mItemAttributes->fingerprint());
                    if(!fp1.isEmpty() && !fp2.isEmpty())
                    {
                        return fp1.compare(fp2) == 0;
                    }
                    else
                    {
                        return check->getUnescapeConflictedName().compare(cloudConflictedName->getUnescapeConflictedName(), Qt::CaseSensitive) == 0;
                    }
                }
                else
                {
                    return false;
                }
            });

            if(counter == 0 && ignoreLastModifiedName)
            {
                cloudConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
                if(localConflictedName != localConflictedNames.end())
                {
                    (*localConflictedName)->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
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

                    if(localConflictedName != localConflictedNames.end())
                    {
                        QFileInfo fileInfo((*localConflictedName)->mConflictedPath);
                        fileInfo.setFile(fileInfo.path(), (*localConflictedName)->getConflictedName());

                        QFile file(fileInfo.filePath());
                        if(file.exists())
                        {
                            fileInfo.setFile(fileInfo.path(), newName);
                            if(file.rename(QDir::toNativeSeparators(fileInfo.filePath())))
                            {
                                (*localConflictedName)->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
                                (*localConflictedName)->mRenameTo = newName;
                            }
                        }
                    }

                    cloudConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
                    cloudConflictedName->mRenameTo = newName;
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
            auto cloudConflictedName = std::find_if(cloudConflictedNames.begin(), cloudConflictedNames.end(), [localConflictedName](const std::shared_ptr<ConflictedNameInfo>& check){

                if(check->mSolved
                   == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
                {
                    auto fp1(check->mItemAttributes->fingerprint());
                    auto fp2(localConflictedName->mItemAttributes->fingerprint());
                    if(!fp1.isEmpty() && !fp2.isEmpty())
                    {
                        return fp1.compare(fp2) == 0;
                    }
                    else
                    {
                        return check->getUnescapeConflictedName().compare(localConflictedName->getUnescapeConflictedName(), Qt::CaseSensitive) == 0;
                    }
                }
                else
                {
                    return false;
                }
            });

            if(counter == 0 && ignoreLastModifiedName)
            {
                localConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;

                if(cloudConflictedName != cloudConflictedNames.end())
                {
                    (*cloudConflictedName)->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
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
                        localConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
                        localConflictedName->mRenameTo = newName;
                    }


                    if(cloudConflictedName != cloudConflictedNames.end())
                    {
                        std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle((*cloudConflictedName)->mHandle));
                        if(conflictedNode)
                        {
                            std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedNode->getParentHandle()));
                            MegaSyncApp->getMegaApi()->renameNode(conflictedNode.get(), newName.toUtf8().constData());

                            (*cloudConflictedName)->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
                            (*cloudConflictedName)->mRenameTo = newName;
                        }
                    }
                }
            }

        }
        counter++;
    }
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
            conflictName->mRenameTo = renameTo;

            result = checkAndSolveConflictedNamesSolved(conflictedNames);
            if(result)
            {
                checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
            }
        }
    }

    return result;
}

bool NameConflictedStalledIssue::checkAndSolveConflictedNamesSolved(const QList<std::shared_ptr<ConflictedNameInfo>>& conflicts)
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

    if(!mIsSolved && unsolvedItems == 0)
    {
        mIsSolved = true;
        MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());
    }

    return mIsSolved;
}

void NameConflictedStalledIssue::solveIssue(bool)
{
   mCloudConflictedNames.removeDuplicatedNodes();
   renameNodesAutomatically();

   auto cloudConflictedNames(mCloudConflictedNames.getConflictedNames());

   auto result = checkAndSolveConflictedNamesSolved(cloudConflictedNames);
   if(result)
   {
       checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
   }
}

