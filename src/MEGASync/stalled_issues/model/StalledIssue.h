#ifndef STALLEDISSUE_H
#define STALLEDISSUE_H

#include <megaapi.h>

#include <QSharedData>
#include <QObject>
#include <QFileInfo>

#include <memory>

enum class StalledIssueFilterCriterion
{
    ALL_ISSUES = 0,
    NAME_CONFLICTS,
    ITEM_TYPE_CONFLICTS,
    OTHER_CONFLICTS,
};

class StalledIssueData : public QSharedData
{
public:
    struct Path
    {
        QString path;
        mega::MegaSyncStall::SyncPathProblem mPathProblem = mega::MegaSyncStall::SyncPathProblem::NoProblem;

        Path(){}
        bool isEmpty() const {return path.isEmpty() && mPathProblem == mega::MegaSyncStall::SyncPathProblem::NoProblem;}
    };

    StalledIssueData();
    ~StalledIssueData(){}

    const Path& getPath() const;
    const Path& getMovePath() const;
    bool isCloud() const;

    QString getFilePath() const;
    QString getMoveFilePath() const;

    QString getNativeFilePath() const;
    QString getNativeMoveFilePath() const;

    QString getNativePath() const;
    QString getNativeMovePath() const;

    QString getFileName() const;

    bool isEqual(const mega::MegaSyncStall *stall) const;

    bool isSolved() const;
    void setIsSolved(bool newIsSolved);

private:
    friend class StalledIssue;
    friend class NameConflictedStalledIssue;

    Path mMovePath;
    Path mPath;

    bool mIsCloud;
    bool mIsSolved;
};

Q_DECLARE_TYPEINFO(StalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(StalledIssueData)

using StalledIssueDataPtr = QExplicitlySharedDataPointer<const StalledIssueData>;
using StalledIssuesDataList = QList<StalledIssueDataPtr>;

Q_DECLARE_METATYPE(StalledIssueDataPtr)
Q_DECLARE_METATYPE(StalledIssuesDataList)

class StalledIssue
{
public:
    StalledIssue(){}
    StalledIssue(const StalledIssue& tdr) : mDetectedMEGASide(tdr.mDetectedMEGASide), mLocalData(tdr.mLocalData), mCloudData(tdr.mCloudData), mReason(tdr.getReason()), mIsSolved(tdr.mIsSolved)  {}
    StalledIssue(const mega::MegaSyncStall *stallIssue);

    const StalledIssueDataPtr consultLocalData() const;
    const StalledIssueDataPtr consultCloudData() const;

    mega::MegaSyncStall::SyncStallReason getReason() const;
    QString getFileName() const;
    static StalledIssueFilterCriterion getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason);

    bool operator==(const StalledIssue &data);

    virtual void updateIssue(const mega::MegaSyncStall *stallIssue);

    bool isSolved() const;
    void setIsSolved(bool isCloud);

    bool canBeIgnored() const;
    QStringList getIgnoredFiles() const;

    bool mDetectedMEGASide = false;

    uint8_t hasFiles() const;
    uint8_t hasFolders() const;

protected:
    bool initCloudIssue();
    const QExplicitlySharedDataPointer<StalledIssueData>& getLocalData() const;
    QExplicitlySharedDataPointer<StalledIssueData> mLocalData;

    bool initLocalIssue();
    const QExplicitlySharedDataPointer<StalledIssueData>& getCloudData() const;
    QExplicitlySharedDataPointer<StalledIssueData> mCloudData;

    void setIsFile(const QString& path, bool isLocal);

    virtual void fillIssue(const mega::MegaSyncStall *stall);

    mega::MegaSyncStall::SyncStallReason mReason = mega::MegaSyncStall::SyncStallReason::NoReason;
    bool mIsSolved = false;
    uint8_t mFiles = 0;
    uint8_t mFolders = 0;
    QStringList mIgnoredPaths;
};

Q_DECLARE_METATYPE(StalledIssue)

class NameConflictedStalledIssue : public StalledIssue
{
public:
    struct ConflictedNameInfo
    {
        enum class SolvedType
        {
            REMOVE = 0,
            RENAME,
            SOLVED_BY_OTHER_SIDE,
            UNSOLVED
        };

        QString conflictedName;
        QString renameTo;
        SolvedType solved;

        ConflictedNameInfo(const QString& name):conflictedName(name),solved(SolvedType::UNSOLVED){}
        bool operator==(const ConflictedNameInfo &data)
        {
            return conflictedName == data.conflictedName;
        }
        bool isSolved() const {return solved != SolvedType::UNSOLVED;}
    };

    struct NameConflictData
    {
        //Not const?? Depending on who is the liable to update the GUI -> From the SDK or from the MEGASync itself?
        StalledIssueDataPtr data;
        QList<ConflictedNameInfo> conflictedNames;
        bool isCloud;

        bool isEmpty() const { return conflictedNames.isEmpty();}
    };

    NameConflictedStalledIssue(){}
    NameConflictedStalledIssue(const NameConflictedStalledIssue& tdr);
    NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue);

    void fillIssue(const mega::MegaSyncStall *stall) override;

    NameConflictData getNameConflictLocalData() const;
    NameConflictData getNameConflictCloudData() const;

    bool solveLocalConflictedName(const QString& name, ConflictedNameInfo::SolvedType type);
    bool solveCloudConflictedName(const QString& name, ConflictedNameInfo::SolvedType type);

    bool solveLocalConflictedNameByRename(const QString& name, const QString& renameTo);
    bool solveCloudConflictedNameByRename(const QString& name, const QString& renameTo);

    bool areLocalAllConflictedNamesSolved();
    bool areCloudAllConflictedNamesSolved();

    static QStringList convertConflictedNames(bool cloud, const mega::MegaSyncStall *stall);

    void updateIssue(const mega::MegaSyncStall *stallIssue) override;

private:
    using StalledIssue::getLocalData;
    using StalledIssue::getCloudData;

    QList<ConflictedNameInfo> mCloudConflictedNames;
    QList<ConflictedNameInfo> mLocalConflictedNames;

};

Q_DECLARE_METATYPE(NameConflictedStalledIssue)

class StalledIssueVariant
{
public:
    StalledIssueVariant(){}
    StalledIssueVariant(const StalledIssueVariant& tdr) : mData(tdr.mData) {}
    StalledIssueVariant(const std::shared_ptr<StalledIssue> data)
        : mData(data)
    {}

    const std::shared_ptr<const StalledIssue> consultData() const
    {
        return mData;
    }

    void updateData(const mega::MegaSyncStall *stallIssue)
    {
        mData->updateIssue(stallIssue);
    }

    bool operator==(const StalledIssueVariant &issue)
    {
        return issue.mData == this->mData;
    }

    StalledIssueVariant& operator=(const StalledIssueVariant& other) = default;

private:
    friend class StalledIssuesModel;

    const std::shared_ptr<StalledIssue> &getData() const
    {
        return mData;
    }

    std::shared_ptr<StalledIssue> mData;
};

Q_DECLARE_METATYPE(StalledIssueVariant)

using StalledIssuesVariantList = QList<std::shared_ptr<StalledIssueVariant>>;
Q_DECLARE_METATYPE(StalledIssuesVariantList)

#endif // STALLEDISSUE_H
