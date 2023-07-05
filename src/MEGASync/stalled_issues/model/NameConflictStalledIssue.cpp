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

            std::shared_ptr<ConflictedNameInfo> info(new ConflictedNameInfo(localPath, std::make_shared<LocalFileFolderAttributes>(getLocalData()->getNativeFilePath(), nullptr)));
            mLocalConflictedNames.append(info);

            setIsFile(localPath.filePath(), true);
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
                std::shared_ptr<ConflictedNameInfo> info(new ConflictedNameInfo(cloudPathInfo, std::make_shared<RemoteFileFolderAttributes>(node->getHandle(), nullptr)));
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
            }
        }

        //No auto solving for the moment
        //        if(Preferences::instance()->stalledIssueSmartMode() == Preferences::StalledIssuesSmartModeType::Smart)
        //        {
        //            solveIssue();
        //        }
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
        conflictName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;;
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
    auto cloudConflictedNames(mCloudConflictedNames.getConflictedNames());

    std::sort(cloudConflictedNames.begin(), cloudConflictedNames.end(), [](const std::shared_ptr<ConflictedNameInfo>& check1, const std::shared_ptr<ConflictedNameInfo>& check2){
        return check1->mItemAttributes->modifiedTime() > check2->mItemAttributes->modifiedTime();
    });

    auto counter(0);
    QStringList itemsBeingRenamed;
    foreach(auto& cloudConflictedName, cloudConflictedNames)
    {
        if(cloudConflictedName->mSolved
                == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
        {
            if(counter == 0)
            {
                cloudConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
            }
            else
            {
                auto localConflictedName = std::find_if(mLocalConflictedNames.begin(), mLocalConflictedNames.end(), [cloudConflictedName](const std::shared_ptr<ConflictedNameInfo>& check){
                    return check->mConflictedName.compare(cloudConflictedName->mConflictedName, Qt::CaseSensitive) == 0 && check->mSolved
                            == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED;
                });

                std::unique_ptr<mega::MegaNode> conflictedNode(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudConflictedName->mHandle));
                if(conflictedNode)
                {
                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedNode->getParentHandle()));
                    auto newName = Utilities::getNonDuplicatedNodeName(conflictedNode.get(), parentNode.get(), cloudConflictedName->mConflictedName, true, itemsBeingRenamed);
                    MegaSyncApp->getMegaApi()->renameNode(conflictedNode.get(), newName.toUtf8().constData());

                    itemsBeingRenamed.append(newName);

                    if(localConflictedName != mLocalConflictedNames.end())
                    {
                        QFileInfo fileInfo((*localConflictedName)->mConflictedPath);
                        fileInfo.setFile(fileInfo.path(), (*localConflictedName)->mConflictedName);

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

    //Local files
    auto localConflictedNames(mLocalConflictedNames);
    std::sort(localConflictedNames.begin(), localConflictedNames.end(), [](const std::shared_ptr<ConflictedNameInfo>& check1, const std::shared_ptr<ConflictedNameInfo>& check2){
        return check1->mItemAttributes->modifiedTime() > check2->mItemAttributes->modifiedTime();
    });

    counter = 0;

    foreach(auto& localConflictedName, localConflictedNames)
    {
        if(localConflictedName->mSolved
                == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
        {
            if(counter == 0)
            {
                localConflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
            }
            else
            {
                auto cloudConflictedName = std::find_if(cloudConflictedNames.begin(), cloudConflictedNames.end(), [localConflictedName](const std::shared_ptr<ConflictedNameInfo>& check){
                    return check->mConflictedName.compare(localConflictedName->mConflictedName, Qt::CaseSensitive) == 0 && check->mSolved
                            == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED;
                });

                QFileInfo fileInfo(localConflictedName->mConflictedPath);
                fileInfo.setFile(fileInfo.path(), localConflictedName->mConflictedName);

                QFile file(fileInfo.filePath());
                if(file.exists())
                {
                    auto newName = Utilities::getNonDuplicatedLocalName(fileInfo, true);

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

            counter++;
        }
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

    mIsSolved = unsolvedItems == 0;

    if(mIsSolved)
    {
        MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());
    }

    return mIsSolved;
}

void NameConflictedStalledIssue::solveIssue()
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

bool NameConflictedStalledIssue::isSolved() const
{
    return mIsSolved;
}

