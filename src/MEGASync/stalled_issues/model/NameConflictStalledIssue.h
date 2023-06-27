#ifndef NAMECONFLICTSTALLEDISSUE_H
#define NAMECONFLICTSTALLEDISSUE_H

#include <StalledIssue.h>
#include <MegaApplication.h>
#include <FileFolderAttributes.h>
#include <StalledIssuesUtilities.h>

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
            REMOVE_AUTOMATICALLY,
            UNSOLVED
        };

        enum class DuplicatedType
        {
            NO_DUPLICATED,
            KEEP,
            REMOVABLE
        };

        mega::MegaHandle mHandle;
        QString mConflictedName;
        QString mConflictedPath;
        QString mRenameTo;
        SolvedType mSolved;
        DuplicatedType mDuplicated;
        int mDuplicatedGroupId;
        std::shared_ptr<FileFolderAttributes>  mItemAttributes;

        ConflictedNameInfo()
            : mSolved(SolvedType::UNSOLVED)
        {}

        ConflictedNameInfo(const QFileInfo& fileInfo, std::shared_ptr<FileFolderAttributes> attributes)
            :mConflictedName(fileInfo.fileName()),
              mConflictedPath(fileInfo.filePath()),
              mSolved(SolvedType::UNSOLVED),
              mDuplicated(DuplicatedType::NO_DUPLICATED),
              mDuplicatedGroupId(-1),
              mItemAttributes(attributes)
        {}

        bool operator==(const ConflictedNameInfo &data)
        {
            return mConflictedName == data.mConflictedName;
        }
        bool isSolved() const {return mSolved != SolvedType::UNSOLVED;}
    };

    class CloudConflictedNames
    {
    public:
        CloudConflictedNames(int64_t utimestamp, QString ufingerprint)
            : timestamp(utimestamp), fingerprint(ufingerprint)
        {}

        int64_t timestamp;
        QString fingerprint;

        QList<std::shared_ptr<ConflictedNameInfo>> conflictedNames;
    };

    class CloudConflictedNamesByHandle
    {
    public:
        CloudConflictedNamesByHandle()
        {}

        void addFolderConflictedName(int64_t timestamp, QString fingerprint, std::shared_ptr<ConflictedNameInfo> info)
        {
            CloudConflictedNames newConflictedName(timestamp, fingerprint);
            newConflictedName.conflictedNames.append(info);
            mConflictedNames.append(newConflictedName);
        }

        void addFileConflictedName(int64_t timestamp, QString fingerprint, std::shared_ptr<ConflictedNameInfo> info)
        {
            for(int index = 0; index < mConflictedNames.size(); ++index)
            {
                auto& namesByHandle = mConflictedNames[index];
                if(namesByHandle.timestamp == timestamp
                        && fingerprint == namesByHandle.fingerprint)
                {
                    auto previousSize = namesByHandle.conflictedNames.size();

                    if(previousSize >= 1)
                    {
                        if(previousSize == 1)
                        {
                            auto firstNameByHandle(namesByHandle.conflictedNames.first());
                            firstNameByHandle->mDuplicated = ConflictedNameInfo::DuplicatedType::KEEP;
                            firstNameByHandle->mDuplicatedGroupId = index;
                        }

                        info->mDuplicated = ConflictedNameInfo::DuplicatedType::REMOVABLE;
                        info->mDuplicatedGroupId = index;
                    }

                    namesByHandle.conflictedNames.append(info);
                    return;
                }
            }

            CloudConflictedNames newConflictedName(timestamp, fingerprint);
            newConflictedName.conflictedNames.append(info);
            mConflictedNames.append(newConflictedName);
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
                    aux.append(namesByHandle.conflictedNames);
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

        void removeDuplicatedNodes(int groupIndex)
        {
            if(mConflictedNames.size() > groupIndex)
            {
                StalledIssuesUtilities utilities;
                auto& conflictedNamesGroup = mConflictedNames[groupIndex];
                if(conflictedNamesGroup.conflictedNames.size() > 1)
                {
                    int counter(0);
                    foreach(auto conflictedName, conflictedNamesGroup.conflictedNames)
                    {
                        if(counter > 0)
                        {
                            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(conflictedName->mHandle));
                            if(node)
                            {
                                //conflictedName->mSolved = ConflictedNameInfo::SolvedType::REMOVE_AUTOMATICALLY;
                                utilities.removeRemoteFile(node.get());
                                conflictedNamesGroup.conflictedNames.removeOne(conflictedName);
                            }
                        }

                        counter++;
                    }
                }
            }
        }

    private:
         QList<CloudConflictedNames> mConflictedNames;
    };

    NameConflictedStalledIssue(){}
    NameConflictedStalledIssue(const NameConflictedStalledIssue& tdr);
    NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue);

    void fillIssue(const mega::MegaSyncStall *stall) override;

    const QList<std::shared_ptr<ConflictedNameInfo>>& getNameConflictLocalData() const;
    const CloudConflictedNamesByHandle& getNameConflictCloudData() const;

    bool solveLocalConflictedName(int conflictIndex, ConflictedNameInfo::SolvedType type);
    bool solveCloudConflictedName(int conflictIndex, ConflictedNameInfo::SolvedType type);

    bool solveCloudConflictedNameByRename(int conflictIndex, const QString& renameTo);
    bool solveLocalConflictedNameByRename(int conflictIndex, const QString& renameTo);

    void updateIssue(const mega::MegaSyncStall *stallIssue) override;

    void solveIssue();
    bool isSolved() const;

private:
    bool checkAndSolveConflictedNamesSolved(const QList<std::shared_ptr<ConflictedNameInfo> > &conflicts);

    CloudConflictedNamesByHandle mCloudConflictedNames;
    QList<std::shared_ptr<ConflictedNameInfo>> mLocalConflictedNames;
    mutable bool mIsSolved = false;
};

Q_DECLARE_METATYPE(NameConflictedStalledIssue)

#endif // NAMECONFLICTSTALLEDISSUE_H
