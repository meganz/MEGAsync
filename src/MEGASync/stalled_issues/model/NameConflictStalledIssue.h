#ifndef NAMECONFLICTSTALLEDISSUE_H
#define NAMECONFLICTSTALLEDISSUE_H

#include <StalledIssue.h>
#include <MegaApplication.h>
#include <FileFolderAttributes.h>
#include <StalledIssuesUtilities.h>
#include <Utilities.h>
#include <MegaNodeNames.h>

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
            FAILED,
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
        bool mDuplicated;
        int mDuplicatedGroupId;
        bool mIsFile;
        std::shared_ptr<FileFolderAttributes>  mItemAttributes;
        std::shared_ptr<mega::MegaError> mError;
        QString mErrorContext;

        ConflictedNameInfo(const QFileInfo& fileInfo,
            bool isFile,
            std::shared_ptr<FileFolderAttributes> attributes)
            : mConflictedName(fileInfo.fileName())
            , mHandle(mega::INVALID_HANDLE)
            , mConflictedPath(fileInfo.filePath())
            , mDuplicatedGroupId(-1)
            , mDuplicated(false)
            , mIsFile(isFile)
            , mItemAttributes(attributes)
            , mSolved(SolvedType::UNSOLVED)
        {
        }

        bool operator==(const ConflictedNameInfo &data)
        {
            return mConflictedName == data.mConflictedName;
        }
        bool isSolved() const {return mSolved < SolvedType::FAILED;}
        bool isFailed() const {return mSolved == SolvedType::FAILED;}

        void checkExternalChange()
        {
            if(mSolved == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED)
            {
                if(mHandle != mega::INVALID_HANDLE)
                {
                    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
                    if(node && node->isNodeKeyDecrypted())
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

            mError.reset();
        }

        void solveByRemove()
        {
            mSolved = ConflictedNameInfo::SolvedType::REMOVE;
            mError.reset();
        }

        void setFailed(std::shared_ptr<mega::MegaError> error, const QString& context = QString())
        {
            mError = error;
            mErrorContext = context;
            if(error || !context.isEmpty())
            {
                mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::FAILED;
            }
            else
            {
                mSolved = NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED;
            }
        }

        void solveByMerge()
        {
            mSolved = ConflictedNameInfo::SolvedType::MERGED;
            mError.reset();
        }

        void solveByOtherSide()
        {
            mSolved = ConflictedNameInfo::SolvedType::SOLVED_BY_OTHER_SIDE;
        }

        SolvedType getSolvedType() const
        {
            return mSolved;
        }

    private:
        QString mConflictedName;
        QString mUnescapedConflictedName;
        SolvedType mSolved;
    };

    struct CloudConflictedNameAttributes
    {
        QString fingerprint;
        int64_t size = -1;
        int64_t modifiedTime = -1;
    };

    class CloudConflictedNamesByAttributes
    {
    public:
        CloudConflictedNamesByAttributes(QString ufingerprint, int64_t usize, int64_t umodifiedTime)
        {
            mAttributes.fingerprint = ufingerprint;
            mAttributes.size = usize;
            mAttributes.modifiedTime = umodifiedTime;
        }

        CloudConflictedNamesByAttributes()
        {}

        CloudConflictedNameAttributes mAttributes;

        bool solved = false;

        QMultiMap<int64_t, std::shared_ptr<ConflictedNameInfo>> conflictedNames;
    };

    class CloudConflictedNames
    {
    public:
        CloudConflictedNames()
        {}

        void addFolderConflictedName(mega::MegaHandle handle, std::shared_ptr<ConflictedNameInfo> info)
        {
            CloudConflictedNamesByAttributes newConflictedName;
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

                if(fingerprint == namesByHandle.mAttributes.fingerprint
                   && size == namesByHandle.mAttributes.size
                   && modifiedtimestamp == namesByHandle.mAttributes.modifiedTime)
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

                CloudConflictedNamesByAttributes newConflictedName(fingerprint, size, modifiedtimestamp);
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
                if(fingerprint == namesByHandle.mAttributes.fingerprint
                        && size == namesByHandle.mAttributes.size
                        && modifiedtimestamp == namesByHandle.mAttributes.modifiedTime)
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

            CloudConflictedNamesByAttributes newConflictedName(fingerprint, size, modifiedtimestamp);
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

        CloudConflictedNameAttributes getAttributesByNameInfo(const std::shared_ptr<ConflictedNameInfo>& info) const
        {
            CloudConflictedNameAttributes attributes;

            foreach(auto& namesByAttributes, mConflictedNames)
            {
                if(!namesByAttributes.conflictedNames.isEmpty())
                {
                    auto infoFound = std::find_if(namesByAttributes.conflictedNames.begin(), namesByAttributes.conflictedNames.end(),
                                                  [&info](const std::shared_ptr<ConflictedNameInfo>& check){
                        return info == check;
                    });

                    if(infoFound != namesByAttributes.conflictedNames.end())
                    {
                        return namesByAttributes.mAttributes;
                    }
                }
            }

            return attributes;
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

        QString getConflictedName(const std::shared_ptr<ConflictedNameInfo>& info) const
        {
            if(!mConflictedNames.isEmpty())
            {
                if(isKeyUndecryped(info))
                {
                    return MegaNodeNames::getUndecryptedName();
                }

                return info->getConflictedName();
            }

            return QString();
        }

        QString getConflictedName()
        {
            if(!mConflictedNames.isEmpty() && !mConflictedNames.first().conflictedNames.isEmpty())
            {
                auto firstConflictedNameInfo(mConflictedNames.first().conflictedNames.first());

                if(isKeyUndecryped(firstConflictedNameInfo))
                {
                    return MegaNodeNames::getUndecryptedName();
                }

                return firstConflictedNameInfo->getConflictedName();
            }

            return QString();
        }

        bool areAllKeysDecrypted() const
        {
            foreach(auto& namesByAttributes, mConflictedNames)
            {
                foreach(auto& conflictedName, namesByAttributes.conflictedNames)
                {
                    if(isKeyUndecryped(conflictedName))
                    {
                        return false;
                    }
                }
            }

            return true;
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

        std::shared_ptr<mega::MegaError> removeDuplicatedNodes()
        {
            MoveToCloudBinUtilities utilities;

            for(int index = 0; index < mConflictedNames.size(); ++index)
            {
                auto& conflictedNamesGroup = mConflictedNames[index];

                if(conflictedNamesGroup.conflictedNames.size() > 1)
                {
                    //The object is auto deleted when finished (as it needs to survive this issue)
                    foreach(auto conflictedName, conflictedNamesGroup.conflictedNames)
                    {
                        if(conflictedName->getSolvedType() == NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::UNSOLVED &&
                           conflictedName != (*(conflictedNamesGroup.conflictedNames.end()-1)))
                        {
                            auto moveToBinErrors = utilities.moveToBin(conflictedName->mHandle, QLatin1String("SyncDuplicated"), true);
                            if(!moveToBinErrors.binFolderCreationError && !moveToBinErrors.moveError)
                            {
                                conflictedName->solveByRemove();
                            }
                            else
                            {
                                if(moveToBinErrors.binFolderCreationError)
                                {
                                    conflictedName->setFailed(
                                        moveToBinErrors.binFolderCreationError, tr("Bin folder could not be created."));

                                    return moveToBinErrors.binFolderCreationError;
                                }
                                else
                                {
                                    QString errorContext = conflictedName->mIsFile ? tr("File could not be moved.") : tr("Folder could not be moved");

                                    conflictedName->setFailed(moveToBinErrors.moveError, errorContext);

                                    return moveToBinErrors.moveError;
                                }
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

        struct MergeFoldersError
        {
            std::shared_ptr<mega::MegaError> error;
            QString errorContext;
            int conflictIndex;
        };

        MergeFoldersError mergeFolders()
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
                        CloudFoldersMerge mergeItem(targetFolder.get(), folderToMerge.get());
                        auto error = mergeItem.merge(CloudFoldersMerge::ActionForDuplicates::IgnoreAndMoveToBin);
                        if(error)
                        {
                            errorInfo.error = error;
                            errorInfo.conflictIndex = index;
                            errorInfo.errorContext = tr("Error merging folder.");
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

    private:
        bool isKeyUndecryped(const std::shared_ptr<ConflictedNameInfo>& info) const
        {
            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(info->mHandle));
            if(node && !node->isNodeKeyDecrypted())
            {
                return true;
            }

            return false;
        }

        QList<CloudConflictedNamesByAttributes> mConflictedNames;
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
    const CloudConflictedNames& getNameConflictCloudData() const;

    void setCloudFailed(int errorConflictIndex, std::shared_ptr<mega::MegaError> error, const QString& errorContext);
    void setLocalFailed(int errorConflictIndex, const QString& error);

    bool containsHandle(mega::MegaHandle handle) override;
    void updateHandle(mega::MegaHandle handle) override;
    void updateName() override;

    bool checkForExternalChanges() override;

    bool solveLocalConflictedNameByRemove(int conflictIndex);
    bool solveCloudConflictedNameByRemove(int conflictIndex);

    bool solveCloudConflictedNameByRename(int conflictIndex, const QString& renameTo);
    bool solveLocalConflictedNameByRename(int conflictIndex, const QString& renameTo);

    bool hasFoldersToMerge() const;

    bool renameNodesAutomatically();

    void semiAutoSolveIssue(int option);
    bool autoSolveIssue() override;

    bool hasDuplicatedNodes() const;
    bool areAllDuplicatedNodes() const;

    void updateIssue(const mega::MegaSyncStall *stallIssue) override;

    QStringList getLocalFiles() override;

    bool UIShowFileAttributes() const override
    {
        return true;
    }

    bool shouldBeIgnored() const override;

    static void showLocalRenameHasFailedMessageBox(const QString& itemName, bool isFile);
    static void showRemoteRenameHasFailedMessageBox(const mega::MegaError& error, bool isFile);

private:
    bool checkAndSolveConflictedNamesSolved(bool isPotentiallySolved = false);

    void solveIssue(int option);

    bool renameCloudNodesAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                       const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                       bool ignoreLastModifiedName,
                                       QStringList &cloudItemsBeingRenamed);
    bool renameLocalItemsAutomatically(const QList<std::shared_ptr<ConflictedNameInfo>>& cloudConflictedNames,
                                       const QList<std::shared_ptr<ConflictedNameInfo>>& localConflictedNames,
                                       bool ignoreLastModifiedName,
                                       QStringList &cloudItemsBeingRenamed);

    //Rename siblings
    bool renameCloudSibling(std::shared_ptr<ConflictedNameInfo> item, const QString& newName);
    bool renameLocalSibling(std::shared_ptr<ConflictedNameInfo> item, const QString& newName);

    //Find local or remote sibling
    std::shared_ptr<ConflictedNameInfo> findOtherSideItem(const QList<std::shared_ptr<ConflictedNameInfo>>& items, std::shared_ptr<ConflictedNameInfo> check);

    CloudConflictedNames mCloudConflictedNames;
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
