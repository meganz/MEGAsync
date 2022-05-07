#ifndef STALLEDISSUE_H
#define STALLEDISSUE_H

#include <megaapi.h>

#include <QSharedData>
#include <QObject>

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
        bool isMissing;
        bool isBlocked;
        QString path;

        Path() : isMissing(false), isBlocked(false){}
        bool isEmpty() const {return path.isEmpty();}
    };

    StalledIssueData(const mega::MegaSyncStall* stallIssue = nullptr);
    ~StalledIssueData(){}

    Path mMovePath;
    Path mPath;

    bool mIsCloud;
    bool mIsImmediate;
    QString mReasonString;

    bool hasMoveInfo() const;

private:
    friend class StalledIssue;

    void update(const mega::MegaSyncStall* stallIssue);
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
        StalledIssue(const StalledIssue& tdr) : d(tdr.d), mReason(tdr.getReason()), mIsNameConflict(tdr.isNameConflict()), mFileName(tdr.getFileName()) {}
        StalledIssue(const mega::MegaSyncStall *stallIssue);
        //StalledIssue(const QList<QExplicitlySharedDataPointer<StalledIssueData>>& tdr, mega::MegaSyncStall::SyncStallReason reason = mega::MegaSyncStall::SyncStallReason::NoReason);

        void addStalledIssueData(const mega::MegaSyncStall *stallIssue);
        QExplicitlySharedDataPointer<StalledIssueData> getStalledIssueData(int index = 0) const;
        int stalledIssuesCount() const;

        mega::MegaSyncStall::SyncStallReason getReason() const;
        const QString& getFileName() const;
        bool isCloud() const;

        bool isNameConflict() const;

        bool operator==(const StalledIssue &data);

        static StalledIssueFilterCriterion getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason);

protected:
        QList<QExplicitlySharedDataPointer<StalledIssueData>> d;
        mega::MegaSyncStall::SyncStallReason mReason = mega::MegaSyncStall::SyncStallReason::NoReason;
        bool mIsNameConflict = false;
        QString mFileName;

private:
        void extractFileName(const mega::MegaSyncStall *stall);
        void fillIssue(const mega::MegaSyncStall *stall);
        void addStalledIssueData(QExplicitlySharedDataPointer<StalledIssueData> data);
};
Q_DECLARE_METATYPE(StalledIssue)

class ConflictedNamesStalledIssue : public StalledIssue
{
public:
    ConflictedNamesStalledIssue();
    ConflictedNamesStalledIssue(const QExplicitlySharedDataPointer<StalledIssueData>& tdr);
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
