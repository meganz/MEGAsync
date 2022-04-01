#include "StalledIssue.h"

#include "MegaApplication.h"

void StalledIssueData::update(mega::MegaSyncStall *stallIssue)
{
    auto megaApi = MegaSyncApp->getMegaApi();
    if(stallIssue && megaApi)
    {
        mIndexPath    = QString::fromUtf8(stallIssue->indexPath());
        mLocalPath    = QString::fromUtf8(stallIssue->localPath());
        mCloudPath    = QString::fromUtf8(stallIssue->cloudPath());
        mReason       = stallIssue->reason();
        mIsCloud      = stallIssue->isCloud();
        mIsImmediate  = stallIssue->isImmediate();
        mReasonString = QString::fromUtf8(stallIssue->reasonString());
    }
}
