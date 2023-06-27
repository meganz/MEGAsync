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

        std::shared_ptr<mega::MegaNode> parentNode(nullptr);

        auto firstCloudPath(stall->path(true,0));
        std::unique_ptr<mega::MegaNode> firstNode(MegaSyncApp->getMegaApi()->getNodeByPath(firstCloudPath));
        if(firstNode)
        {
            if(!parentNode)
            {
                parentNode.reset(MegaSyncApp->getMegaApi()->getNodeByHandle(firstNode->getParentHandle()));
            }

            if(consultCloudData()->mPath.isEmpty())
            {
                QFileInfo cloudPathInfo(QString::fromUtf8(firstCloudPath));
                //We set the first path, as it will be used to get the folder path (discarding the filename)
                getCloudData()->mPath.path = cloudPathInfo.filePath();
            }
        }

        std::unique_ptr<mega::MegaNodeList> nodeList(MegaSyncApp->getMegaApi()->getChildren(parentNode.get(), mega::MegaApi::ORDER_CREATION_DESC));
        QList<int> removedIndexes;

        for(int nodeIndex = 0; nodeIndex < nodeList->size(); ++nodeIndex)
        {
            if(removedIndexes.contains(nodeIndex))
            {
                continue;
            }

            auto node(nodeList->get(nodeIndex));
            if(node)
            {
                for(unsigned int index = 0; index < cloudConflictNames; ++index)
                {
                    QString cloudPath = QString::fromUtf8(stall->path(true,index));
                    QFileInfo cloudPathInfo(cloudPath);

                    QString nodeName(QString::fromUtf8(node->getName()));
                    if(nodeName.compare(cloudPathInfo.fileName(),Qt::CaseSensitive) == 0)
                    {
                        std::shared_ptr<ConflictedNameInfo> info(new ConflictedNameInfo(cloudPathInfo, std::make_shared<RemoteFileFolderAttributes>(node->getHandle(), nullptr)));
                        info->mHandle = node->getHandle();

                        if(node->isFile())
                        {
                            mCloudConflictedNames.addFileConflictedName(node->getModificationTime(), QString::fromUtf8(node->getFingerprint()), info);
                            mFiles++;
                        }
                        else
                        {
                            mCloudConflictedNames.addFolderConflictedName(node->getModificationTime(), QString::fromUtf8(node->getFingerprint()), info);
                            mFolders++;
                        }

                        removedIndexes.append(nodeIndex);

                        break;
                    }
                }
            }
        }

        solveIssue();
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

void NameConflictedStalledIssue::solveIssue()
{
   for(int index = 0; index < mCloudConflictedNames.size(); ++index)
   {
       mCloudConflictedNames.removeDuplicatedNodes(index);
   }

   auto cloudConflictedNames(mCloudConflictedNames.getConflictedNames());

   if(cloudConflictedNames.size() + mLocalConflictedNames.size() <= 2)
   {
       MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());
       mIsSolved = true;
   }
}

const QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>>& NameConflictedStalledIssue::getNameConflictLocalData() const
{
    return mLocalConflictedNames;
}

const NameConflictedStalledIssue::CloudConflictedNamesByHandle& NameConflictedStalledIssue::getNameConflictCloudData() const
{
    return mCloudConflictedNames;
}

bool NameConflictedStalledIssue::solveLocalConflictedName(int conflictIndex, ConflictedNameInfo::SolvedType type)
{
    auto result(false);
    if(mLocalConflictedNames.size() > conflictIndex)
    {
        auto& conflictName = mLocalConflictedNames[conflictIndex];
        conflictName->mSolved = type;

        result = checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mCloudConflictedNames.getConflictedNames());
        }
    }

    return result;
}

bool NameConflictedStalledIssue::solveCloudConflictedName(int conflictIndex, NameConflictedStalledIssue::ConflictedNameInfo::SolvedType type)
{
    auto result(false);

    auto conflictName = mCloudConflictedNames.getConflictedNameByIndex(conflictIndex);
    if(conflictName)
    {
        conflictName->mSolved = type;

        result = checkAndSolveConflictedNamesSolved(mCloudConflictedNames.getConflictedNames());
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
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

bool NameConflictedStalledIssue::solveCloudConflictedNameByRename(int conflictIndex, const QString &renameTo)
{
    auto result(false);

    auto conflictName = mCloudConflictedNames.getConflictedNameByIndex(conflictIndex);
    if(conflictName)
    {
        conflictName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;;
        conflictName->mRenameTo = renameTo;

        result = checkAndSolveConflictedNamesSolved(mCloudConflictedNames.getConflictedNames());
        if(result)
        {
            checkAndSolveConflictedNamesSolved(mLocalConflictedNames);
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

    return unsolvedItems == 0;
}

bool NameConflictedStalledIssue::isSolved() const
{
    return mIsSolved;
}

