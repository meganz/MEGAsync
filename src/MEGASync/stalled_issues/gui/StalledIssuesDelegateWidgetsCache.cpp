#include "StalledIssuesDelegateWidgetsCache.h"

#include "stalled_issues_cases/LocalAndRemoteDifferentWidget.h"
#include "stalled_issues_cases/OtherSideMissingOrBlocked.h"
#include "stalled_issues_cases/StalledIssuesCaseHeaders.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueFilePath.h"

#include "Utilities.h"

StalledIssuesDelegateWidgetsCache::StalledIssuesDelegateWidgetsCache()
{}

void StalledIssuesDelegateWidgetsCache::setProxyModel(StalledIssuesProxyModel *proxyModel)
{
    mProxyModel = proxyModel;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getStalledIssueHeaderWidget(const QModelIndex &index, QWidget *parent, const StalledIssue &data) const
{
    auto row = index.row() % 10;

    auto header = mStalledIssueHeaderWidgets[row];

    if(!header)
    {
        header = createHeaderWidget(index, parent, data);
        header->hide();
    }
    else
    {
        header->updateUi(index, data);
    }

    return header;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, const StalledIssue &data) const
{
    auto row = index.parent().row() % 10;

    auto reason = data.getReason();
    auto& itemsByRowMap = mStalledIssueWidgets[toInt(reason)];
    auto item = itemsByRowMap[row];

    if(!item)
    {
        item = createBodyWidget(index, parent, data);
        item->hide();

        itemsByRowMap.insert(row, item);
    }
    else
    {
        item->updateUi(index, data);
    }

    return item;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueHeaderWidget(const QModelIndex& index, QWidget* parent, const StalledIssue &data) const
{
    return createHeaderWidget(index, parent, data);
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, const StalledIssue& data) const
{
   return createBodyWidget(index, parent, data);
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::createBodyWidget(const QModelIndex &index, QWidget *parent, const StalledIssue &data) const
{
    StalledIssueBaseDelegateWidget* item(nullptr);
    auto reason = data.getReason();

    switch(reason)
    {
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        {
            item = new LocalAndRemoteDifferentWidget(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::FileIssue:
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur:
        case mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning:
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        case mega::MegaSyncStall::SyncStallReason::UploadIssue:
        case mega::MegaSyncStall::SyncStallReason::DownloadIssue:
        case mega::MegaSyncStall::SyncStallReason::CannotCreateFolder:
        case mega::MegaSyncStall::SyncStallReason::CannotPerformDeletion:
        case mega::MegaSyncStall::SyncStallReason::FolderMatchedAgainstFile:
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        default:
        {
            item = new OtherSideMissingOrBlocked(parent);
            break;
        }
    }

    if(item)
    {
        item->updateUi(index, data);
    }

    return item;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::createHeaderWidget(const QModelIndex &index, QWidget *parent, const StalledIssue &data) const
{
    StalledIssueHeader* header(nullptr);

    switch(data.getReason())
    {
        case mega::MegaSyncStall::SyncStallReason::FileIssue:
        {
            header  = new FileIssueHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur:
        {
            header  = new MoveOrRenameCannotOccurHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning:
        {
            header  = new DeleteOrMoveWaitingOnScanningHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        {
            header  = new DeleteWaitingOnMovesHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::UploadIssue:
        {
            header  = new UploadIssueHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::DownloadIssue:
        {
            header  = new DownloadIssueHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CannotCreateFolder:
        {
            header  = new CannotCreateFolderHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CannotPerformDeletion:
        {
            header  = new CannotPerformDeletionHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::FolderMatchedAgainstFile:
        {
            header  = new FolderMatchedAgainstFileHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        {
            header  = new SyncItemExceedsSupoortedTreeDepthHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        {
            header = new LocalAndRemoteChangedSinceLastSyncedStateHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        {
            header  = new LocalAndRemotePreviouslyUnsyncedDifferHeader(parent);
            break;
        }
        default:
        {
            header = new DefaultHeader(parent);
        }
    }

    if(header)
    {
        header->updateUi(index, data);
    }

    return header;
}
