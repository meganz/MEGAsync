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

private:
    friend class StalledIssue;
    friend class NameConflictedStalledIssue;

    Path mMovePath;
    Path mPath;

    bool mIsCloud;
};

Q_DECLARE_TYPEINFO(StalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(StalledIssueData)

using StalledIssueDataPtr = QExplicitlySharedDataPointer<const StalledIssueData>;
using StalledIssuesDataList = QList<StalledIssueDataPtr>;

Q_DECLARE_METATYPE(StalledIssueDataPtr)
Q_DECLARE_METATYPE(StalledIssuesDataList)

class StalledIssueVariant
{
public:
    StalledIssueVariant(){}
    StalledIssueVariant(const StalledIssueVariant& tdr) : mData(tdr.mData) {}
    StalledIssueVariant(const std::shared_ptr<StalledIssue> data)
        : mData(data)
    {}

    const std::shared_ptr<StalledIssue> &data() const
    {
        return mData;
    }

    bool operator==(const StalledIssueVariant &issue)
    {
        return issue.mData == this->mData;
    }

private:
    std::shared_ptr<StalledIssue> mData;
};

Q_DECLARE_METATYPE(StalledIssueVariant)

using StalledIssuesVariantList = QList<StalledIssueVariant>;
Q_DECLARE_METATYPE(StalledIssuesVariantList)

class StalledIssue
{
public:
    StalledIssue(){}
    StalledIssue(const StalledIssue& tdr) : mLocalData(tdr.mLocalData), mCloudData(tdr.mCloudData), mReason(tdr.getReason()) {}
    StalledIssue(const mega::MegaSyncStall *stallIssue);

    //Don´t think it´s going to be more stalled issues than 2 (local and remote)
    const StalledIssueDataPtr consultLocalData() const;
    const StalledIssueDataPtr consultCloudData() const;

    mega::MegaSyncStall::SyncStallReason getReason() const;
    QString getFileName() const;
    static StalledIssueFilterCriterion getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason);

    bool operator==(const StalledIssue &data);

protected:
    bool initCloudIssue();
    const QExplicitlySharedDataPointer<StalledIssueData>& getLocalData() const;
    QExplicitlySharedDataPointer<StalledIssueData> mLocalData;

    bool initLocalIssue();
    const QExplicitlySharedDataPointer<StalledIssueData>& getCloudData() const;
    QExplicitlySharedDataPointer<StalledIssueData> mCloudData;

    virtual void fillIssue(const mega::MegaSyncStall *stall);

    mega::MegaSyncStall::SyncStallReason mReason = mega::MegaSyncStall::SyncStallReason::NoReason;

};

Q_DECLARE_METATYPE(StalledIssue)

class NameConflictedStalledIssue : public StalledIssue
{
public:
    struct NameConflictData
    {
        StalledIssueDataPtr data;
        QStringList conflictedNames;
        bool isCloud;

        bool isEmpty() const { return data == nullptr;}
        ~NameConflictData(){}
    };

    NameConflictedStalledIssue(){}
    NameConflictedStalledIssue(const NameConflictedStalledIssue& tdr);
    NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue);

    void fillIssue(const mega::MegaSyncStall *stall) override;

    NameConflictData getNameConflictLocalData() const;
    NameConflictData getNameConflictCloudData() const;

private:
    using StalledIssue::getLocalData;
    using StalledIssue::getCloudData;

    QStringList mCloudConflictedNames;
    QStringList mLocalConflictedNames;

};

Q_DECLARE_METATYPE(NameConflictedStalledIssue)

#endif // STALLEDISSUE_H
