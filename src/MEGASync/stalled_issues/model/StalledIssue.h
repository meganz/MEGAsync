#ifndef STALLEDISSUE_H
#define STALLEDISSUE_H

#include <megaapi.h>

#include <QSharedData>
#include <QObject>

class StalledIssueData : public QSharedData
{
public:
    StalledIssueData(mega::MegaSyncStall* stallIssue = nullptr){update(stallIssue);}
    ~StalledIssueData(){}

    void update(mega::MegaSyncStall* stallIssue);

private:
    QString mIndexPath;
    QString mLocalPath;
    QString mCloudPath;
    mega::MegaSyncStall::SyncStallReason mReason;
    bool mIsCloud;
    bool mIsImmediate;
    QString mReasonString;

};

Q_DECLARE_TYPEINFO(StalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(StalledIssueData)

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

#endif // STALLEDISSUE_H
