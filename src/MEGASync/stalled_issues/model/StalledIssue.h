#ifndef STALLEDISSUE_H
#define STALLEDISSUE_H

#include <megaapi.h>

#include <QSharedData>
#include <QObject>

class StalledIssueData : public QSharedData
{
public:
    StalledIssueData(const mega::MegaSyncStall* stallIssue = nullptr);
    ~StalledIssueData(){}

    void update(const mega::MegaSyncStall* stallIssue);
    bool isEqual(const StalledIssueData& data);

    QString mIndexPath;
    QString mLocalPath;
    QString mCloudPath;
    mega::MegaSyncStall::SyncStallReason mReason;
    bool mIsCloud;
    bool mIsImmediate;
    QString mReasonString;
    QString mFileName;

    bool mIsNameConflict;
};

Q_DECLARE_TYPEINFO(StalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(StalledIssueData)

class ConflictedNamesStalledIssueData : public StalledIssueData
{
public:
    ConflictedNamesStalledIssueData(mega::MegaSyncNameConflict* stallIssue = nullptr);
    ~ConflictedNamesStalledIssueData(){}

    void update(mega::MegaSyncNameConflict* nameConflictStallIssue);

private:
    QStringList mLocalNames;
    QStringList mCloudNames;
};

Q_DECLARE_TYPEINFO(ConflictedNamesStalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(ConflictedNamesStalledIssueData)

class StalledIssue
{
    public:
        StalledIssue() : d(new StalledIssueData()){}
        StalledIssue(const StalledIssue& tdr) : d(tdr.d) {}
        StalledIssue(const QExplicitlySharedDataPointer<StalledIssueData>& tdr) : d(tdr) {}

        QExplicitlySharedDataPointer<StalledIssueData> getStalledIssueData() const
        {
            return d;
        }

    protected:
        QExplicitlySharedDataPointer<StalledIssueData> d;
};
Q_DECLARE_METATYPE(StalledIssue)

using StalledIssueDataPtr = QExplicitlySharedDataPointer<StalledIssueData>;
using StalledIssuesDataList = QList<StalledIssueDataPtr>;

Q_DECLARE_METATYPE(StalledIssueDataPtr)
Q_DECLARE_METATYPE(StalledIssuesDataList)

#endif // STALLEDISSUE_H
