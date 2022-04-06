#include "StalledIssue.h"

#include "MegaApplication.h"

StalledIssueData::StalledIssueData(const mega::MegaSyncStall *stallIssue)
    : mReason(mega::MegaSyncStall::SyncStallReason::NoReason)
    , mIsCloud(false)
    , mIsImmediate(false)
    , mIsNameConflict(false)
{
    update(stallIssue);
}

void StalledIssueData::update(const mega::MegaSyncStall *stallIssue)
{
    if(stallIssue)
    {
        mIndexPath    = QString::fromUtf8(stallIssue->indexPath());
        mLocalPath    = QString::fromUtf8(stallIssue->localPath());
        mCloudPath    = QString::fromUtf8(stallIssue->cloudPath());
        mReason       = stallIssue->reason();
        mIsCloud      = stallIssue->isCloud();
        mIsImmediate  = stallIssue->isImmediate();
        mReasonString = QString::fromUtf8(stallIssue->reasonString());
        mIsNameConflict = false;

        if(mIsCloud)
        {
            auto splittedIndexPath = mIndexPath.split(QString::fromUtf8("/"));
            mFileName = splittedIndexPath.last();
        }
        else
        {
            auto splittedIndexPath = mIndexPath.split(QString::fromUtf8("\\"));
            mFileName = splittedIndexPath.last();
        }
    }
}

bool StalledIssueData::isEqual(const StalledIssueData &data)
{
    if(mIndexPath == data.mIndexPath && mLocalPath == data.mIndexPath && mCloudPath == data.mCloudPath
            && mReason == data.mReason && mIsCloud == data.mIsCloud && mIsImmediate == data.mIsImmediate
            && mReasonString == data.mReasonString && mIsNameConflict == mIsNameConflict)
    {
        return true;
    }

    return false;
}

//Conflicted Names stalled issue
ConflictedNamesStalledIssueData::ConflictedNamesStalledIssueData(mega::MegaSyncNameConflict *stallIssue)
    :StalledIssueData()
{
    update(stallIssue);
}

void ConflictedNamesStalledIssueData::update(mega::MegaSyncNameConflict* nameConflictStallIssue)
{
    if (mega::MegaStringList* cn = nameConflictStallIssue->cloudNames())
    {
        for (int j = 0; j < cn->size(); ++j)
        {
            mCloudNames.append(QString::fromUtf8(cn->get(j)));
        }
    }
    if (auto cp = nameConflictStallIssue->cloudPath())
    {
        if (cp && *cp)
        {
            mCloudPath = QString::fromUtf8(cp);
        }
    }
    if (mega::MegaStringList* ln = nameConflictStallIssue->localNames())
    {
        for (int j = 0; j < ln->size(); ++j)
        {
            mLocalNames.append(QString::fromUtf8(ln->get(j)));
        }
    }
    if (auto lp = nameConflictStallIssue->localPath())
    {
        if (lp && *lp)
        {
            mLocalPath = QString::fromUtf8(lp);
        }
    }

    mIsNameConflict = true;
}
