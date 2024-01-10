#ifndef NAMECONFLICTSTALLEDISSUE_H
#define NAMECONFLICTSTALLEDISSUE_H

#include <StalledIssue.h>
#include <MegaApplication.h>
#include <FileFolderAttributes.h>
#include <StalledIssuesUtilities.h>
#include <Utilities.h>

class NameConflictedStalledIssue : public StalledIssue
{
public:
    class ConflictedNameInfo
    {
    public:
        enum class SolvedType
        {
            REMOVE = 0,
            RENAME,
            SOLVED_BY_OTHER_SIDE,
            MERGED,
            CHANGED_EXTERNALLY,
            UNSOLVED
        };

        QString getUnescapeConflictedName()
        {
            if(mUnescapedConflictedName.isEmpty())
            {
                mUnescapedConflictedName = QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(getConflictedName().toUtf8().constData()));
            }

            return mUnescapedConflictedName;
        }

        QString getConflictedName()
        {
            return mConflictedName;
        }

        mega::MegaHandle mHandle;
        QString mConflictedPath;
        QString mRenameTo;
        SolvedType mSolved;
        bool mDuplicated;
        int mDuplicatedGroupId;
        bool mIsFile;
        std::shared_ptr<FileFolderAttributes>  mItemAttributes;

        ConflictedNameInfo(const QFileInfo& fileInfo, bool isFile, std::shared_ptr<FileFolderAttributes> attributes)
            : mConflictedName(fileInfo.fileName()),
              mHandle(mega::INVALID_HANDLE),
              mConflictedPath(fileInfo.filePath()),
              mSolved(SolvedType::UNSOLVED),
              mDuplicatedGroupId(-1),
              mDuplicated(false),
              mIsFile(isFile),
              mItemAttributes(attributes)
        {
        }

        bool operator==(const ConflictedNameInfo &data)
        {
            return mConflictedName == data.mConflictedName;
        }
        bool isSolved() const {return mSolved != SolvedType::UNSOLVED;}

        void checkExternalChange()
        {
            if(mSolved == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
            {
                if(mHandle != mega::INVALID_HANDLE)
                {
                    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
                    if(node)
                    {
                        auto nodePath = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePathByNodeHandle(mHandle));
                        if(nodePath != mConflictedPath)
                        {
                            mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::CHANGED_EXTERNALLY;
                        }
                    }
                }
                else
                {
                    QFileInfo adaptedPath(mConflictedPath);
                    if(!adaptedPath.exists())
                    {
                        mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::CHANGED_EXTERNALLY;
                    }
                }
            }
        }

        void solveByRename(const QString& newName)
        {
            mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::RENAME;
            mRenameTo = newName;
            QFileInfo adaptedPath(mConflictedPath);
            adaptedPath.setFile(adaptedPath.path(), newName);
            mConflictedPath = adaptedPath.filePath();

            if(auto localAttributes = std::dynamic_pointer_cast<LocalFileFolderAttributes>(mItemAttributes))
            {
                localAttributes->setPath(mConflictedPath);
            }
        }

    private:
        QString mConflictedName;
        QString mUnescapedConflictedName;
    };

    class CloudConflictedNames
    {
    public:
        CloudConflictedNames(QString ufingerprint, int64_t usize, int64_t umodifiedTime)
            : fingerprint(ufingerprint), size(usize), modifiedTime(umodifiedTime)
        {}

        CloudConflictedNames()
        {}

        QString fingerprint;
        int64_t size = -1;
        int64_t modifiedTime = -1;

        bool solved = false;

        QMultiMap<int64_t, std::shared_ptr<ConflictedNameInfo>> conflictedNames;
    };

    class CloudConflictedNamesByHandle
    {
    public:
        CloudConflictedNamesByHandle()
        {}

        void addFolderConflictedName(mega::MegaHandle handle, std::shared_ptr<ConflictedNameInfo> info)
        {
            CloudConflictedNames newConflictedName;
            newConflictedName.conflictedNames.insert(handle, info);
            mConflictedNames.append(newConflictedName);
        }

        void updateFileConflictedName(int64_t modifiedtimestamp, int64_t size, int64_t oldcreationtimestamp,
                                      int64_t newcreationtimestamp,
                                      QString fingerprint, std::shared_ptr<ConflictedNameInfo> info)
        {
            bool isDuplicated(false);

            if(info->mDuplicated)
            {
                auto& namesByHandle = mConflictedNames[info->mDuplicatedGroupId];
                namesByHandle.conflictedNames.remove(oldcreationtimestamp, info);
            }

            for(int index = 0; index < mConflictedNames.size(); ++index)
            {
                auto& namesByHandle = mConflictedNames[index];

                if(fingerprint == namesByHandle.fingerprint
                   && size == namesByHandle.size
                   && modifiedtimestamp == namesByHandle.modifiedTime)
                {
                    info->mDuplicatedGroupId = index;
                    info->mDuplicated = true;

                    namesByHandle.conflictedNames.insertMulti(newcreationtimestamp, info);

                    isDuplicated = true;
                    break;
                }

            }

            if(!isDuplicated && info->mDuplicated)
            {
                auto& namesByHandle = mConflictedNames[info->mDuplicatedGroupId];
                auto duplicatedIssues = namesByHandle.conflictedNames.size();

                if(duplicatedIssues == 1)
                {
                    auto remainIndex(namesByHandle.conflictedNames.first());
                    remainIndex->mDuplicated = false;
                }

                CloudConflictedNames newConflictedName(fingerprint, size, modifiedtimestamp);
                newConflictedName.conflictedNames.insertMulti(newcreationtimestamp,info);
                mConflictedNames.append(newConflictedName);

                info->mDuplicated = false;
            }
        }

        void addFileConflictedName(int64_t modifiedtimestamp, int64_t size, int64_t creationtimestamp,
                                   QString fingerprint, std::shared_ptr<ConflictedNameInfo> info)
        {
            for(int index = 0; index < mConflictedNames.size(); ++index)
            {
                auto& namesByHandle = mConflictedNames[index];
                if(fingerprint == namesByHandle.fingerprint
                        && size == namesByHandle.size
                        && modifiedtimestamp == namesByHandle.modifiedTime)
                {
                    auto previousSize = namesByHandle.conflictedNames.size();

                    if(previousSize >= 1)
                    {
                        if(previousSize == 1)
                        {
                            auto firstNameByHandle(namesByHandle.conflictedNames.first());
                            firstNameByHandle->mDuplicatedGroupId = index;
                            firstNameByHandle->mDuplicated = true;
                        }

                        info->mDuplicatedGroupId = index;
                        info->mDuplicated = true;
                    }

                    namesByHandle.conflictedNames.insertMulti(creationtimestamp, info);
                    return;
                }
            }

            CloudConflictedNames newConflictedName(fingerprint, size, modifiedtimestamp);
            newConflictedName.conflictedNames.insertMulti(creationtimestamp,info);
            mConflictedNames.append(newConflictedName);
        }

        bool areAllDuplicatedNodes(int index) const
        {
            auto result(true);

            if(!mDuplicatedSolved)
            {
                if(mConflictedNames.size() > index)
                {
                    auto conflictedNames = mConflictedNames.at(index);
                    if(!conflictedNames.solved
                            && conflictedNames.conflictedNames.size() < 2)
                    {
                        result = false;
                    }
                }
            }

            return result;
        }

        bool hasDuplicatedNodes(int index) const
        {
            auto result(false);

            if(!mDuplicatedSolved)
            {
                if(mConflictedNames.size() > index)
                {
                    auto conflictedNames = mConflictedNames.at(index);
                    if(!conflictedNames.solved
                       && conflictedNames.conflictedNames.size() > 1)
                    {
                        result = true;
                    }
                }
            }

            return result;
        }

        std::shared_ptr<ConflictedNameInfo> firstNameConflict() const
        {
            foreach(auto& namesByHandle, mConflictedNames)
            {
                if(!namesByHandle.conflictedNames.isEmpty())
                {
                    return namesByHandle.conflictedNames.first();
                }
            }

            return nullptr;
        }

        std::shared_ptr<ConflictedNameInfo> getConflictedNameByIndex(int index) const
        {
            QList<std::shared_ptr<ConflictedNameInfo>> aux = getConflictedNames();
            if(aux.size() > index)
            {
                return aux.at(index);
            }

            return nullptr;
        }

        QList<std::shared_ptr<ConflictedNameInfo>> getConflictedNames() const
        {
            QList<std::shared_ptr<ConflictedNameInfo>> aux;

            foreach(auto& namesByHandle, mConflictedNames)
            {
                if(!namesByHandle.conflictedNames.isEmpty())
                {
                    aux.append(namesByHandle.conflictedNames.values());
                }
            }

            return aux;
        }

        int size() const
        {
            auto counter(0);
            foreach(auto& conflictedName, mConflictedNames)
            {
                counter += conflictedName.conflictedNames.size();
            }

            return counter;
        }

        void clear()
        {
            mConflictedNames.clear();
        }

        bool isEmpty()
        {
            return mConflictedNames.isEmpty();
        }

        void removeDuplicatedNodes()
        {
            std::unique_ptr<MoveToCloudBinUtilities> utilities(new MoveToCloudBinUtilities());
            QList<mega::MegaHandle> nodesToMove;

            for(int index = 0; index < mConflictedNames.size(); ++index)
            {
                auto& conflictedNamesGroup = mConflictedNames[index];

                if(conflictedNamesGroup.conflictedNames.size() > 1)
                {
                    //The object is auto deleted when finished (as it needs to survive this issue)
                    foreach(auto conflictedName, conflictedNamesGroup.conflictedNames)
                    {
                        if(conflictedName->mSolved == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED &&
                           conflictedName != (*(conflictedNamesGroup.conflictedNames.end()-1)))
                        {
                            conflictedName->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::REMOVE;
                            nodesToMove.append(conflictedName->mHandle);
                        }
                    }

                    conflictedNamesGroup.solved = true;
                }
            }

            utilities->moveToBin(nodesToMove, QLatin1String("SyncDuplicated"), true);
            mDuplicatedSolved = true;
        }

        void mergeFolders()
        {
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
                foreach(auto conflictedFolder, conflictedNames)
                {
                    std::unique_ptr<mega::MegaNode> folderToMerge(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedFolder->mHandle));
                    if(folderToMerge && folderToMerge->isFolder())
                    {
                        CloudFoldersMerge mergeItem(targetFolder.get(), folderToMerge.get());
                        mergeItem.merge(CloudFoldersMerge::ActionForDuplicates::IgnoreAndMoveToBin);
                        conflictedFolder->mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::MERGED;
                    }
                }
            }
        }


    private:
         QList<CloudConflictedNames> mConflictedNames;
         bool mDuplicatedSolved = false;
    };

    enum ActionSelected
    {
        None = 0,
        RemoveDuplicated = 0x01,
        Rename = 0x02,
        MergeFolders = 0x04
    };
    Q_DECLARE_FLAGS(ActionsSelected, ActionSelected)

    NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue);
    ~NameConflictedStalledIssue(){}

    void fillIssue(const mega::MegaSyncStall *stall) override;

    const QList<std::shared_ptr<ConflictedNameInfo>>& getNameConflictLocalData() const;
    const CloudConflictedNamesByHandle& getNameConflictCloudData() const;

    bool containsHandle(mega::MegaHandle handle) override;
    void updateHandle(mega::MegaHandle handle) override;
    void updateName() override;

    bool checkForExternalChanges() override;

    bool solveLocalConflictedNameByRemove(int conflictIndex);
    bool solveCloudConflictedNameByRemove(int conflictIndex);

    bool solveCloudConflictedNameByRename(int conflictIndex, const QString& renameTo);
    bool solveLocalConflictedNameByRename(int conflictIndex, const QString& renameTo);

    void renameNodesAutomatically();

    void semiAutoSolveIssue(int option);
    bool autoSolveIssue() override;
    bool isSolvable() const override;

    bool hasDuplicatedNodes() const;
    bool areAllDuplicatedNodes() const;

    void updateIssue(const mega::MegaSyncStall *stallIssue) override;

    QStringList getLocalFiles() override;

    bool UIShowFileAttributes() const override
    {
        return true;
    }

private:
    bool checkAndSolveConflictedNamesSolved(bool isPotentiallySolved = false);

    void solveIssue(int option);

    void renameCloudNodesAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                       const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                       bool ignoreLastModifiedName,
                                       QStringList &cloudItemsBeingRenamed);
    void renameLocalItemsAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                       const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                       bool ignoreLastModifiedName,
                                       QStringList &cloudItemsBeingRenamed);

    //Rename siblings
    void renameCloudSibling(std::shared_ptr<ConflictedNameInfo> item, const QString& newName);
    void renameLocalSibling(std::shared_ptr<ConflictedNameInfo> item, const QString& newName);

    //Find local or remote sibling
    std::shared_ptr<ConflictedNameInfo> findOtherSideItem(const QList<std::shared_ptr<ConflictedNameInfo>>& items, std::shared_ptr<ConflictedNameInfo> check);

    CloudConflictedNamesByHandle mCloudConflictedNames;
    QList<std::shared_ptr<ConflictedNameInfo>> mLocalConflictedNames;

    //Convenient class
    //Last name conflict node that has been modified (handle must be updated)
    struct LastNodeModified
    {
        std::shared_ptr<ConflictedNameInfo> cloudItem;
        bool usedAsCloudDataHandle = false;

        bool isValid(){return cloudItem != nullptr;}
    };
    LastNodeModified mLastModifiedNode;
};

#endif // NAMECONFLICTSTALLEDISSUE_H
