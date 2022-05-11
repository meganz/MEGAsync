#ifndef STALLEDISSUE_H
#define STALLEDISSUE_H

#include <megaapi.h>

#include <QSharedData>
#include <QObject>
#include <QFileInfo>

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
    friend class ConflictedNamesStalledIssue;

    Path mMovePath;
    Path mPath;

    bool mIsCloud;
};

Q_DECLARE_TYPEINFO(StalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(StalledIssueData)

using StalledIssueDataPtr = QExplicitlySharedDataPointer<StalledIssueData>;
using StalledIssuesDataList = QList<StalledIssueDataPtr>;

Q_DECLARE_METATYPE(StalledIssueDataPtr)
Q_DECLARE_METATYPE(StalledIssuesDataList)

class StalledIssue
{
    public:
        StalledIssue(){}
        StalledIssue(const StalledIssue& tdr) : mLocalData(tdr.mLocalData), mCloudData(tdr.mCloudData), mReason(tdr.getReason()), mIsNameConflict(tdr.isNameConflict()) {}
        StalledIssue(const mega::MegaSyncStall *stallIssue);

        void fillIssue(const mega::MegaSyncStall *stall);

        //Don´t think it´s going to be more stalled issues than 2 (local and remote)
        const QExplicitlySharedDataPointer<StalledIssueData>& getLocalData() const;
        const QExplicitlySharedDataPointer<StalledIssueData>& getCloudData() const;

        mega::MegaSyncStall::SyncStallReason getReason() const;

        QString getFileName() const;

        bool isNameConflict() const;

        bool operator==(const StalledIssue &data);

        static StalledIssueFilterCriterion getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason);

protected:
        bool initLocalIssue();
        QExplicitlySharedDataPointer<StalledIssueData> mCloudData;

        bool initCloudIssue();
        QExplicitlySharedDataPointer<StalledIssueData> mLocalData;

        mega::MegaSyncStall::SyncStallReason mReason = mega::MegaSyncStall::SyncStallReason::NoReason;

        bool mIsNameConflict = false;

};
Q_DECLARE_METATYPE(StalledIssue)

class ConflictedNamesStalledIssue : public StalledIssue
{
public:
    ConflictedNamesStalledIssue();
    ConflictedNamesStalledIssue(mega::MegaSyncNameConflict* nameConflictStallIssue);

    ~ConflictedNamesStalledIssue(){}

    void update(const mega::MegaSyncNameConflict* stallIssue);

    QStringList localNames() const;
    QStringList cloudNames() const;

private:
    QStringList mLocalNames;
    QStringList mCloudNames;
};

//Q_DECLARE_TYPEINFO(ConflictedNamesStalledIssue, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(ConflictedNamesStalledIssue)

using StalledIssuesList = QList<StalledIssue>;
Q_DECLARE_METATYPE(StalledIssuesList)

#endif // STALLEDISSUE_H
