#include "StalledIssuesDelegateWidgetsCache.h"

#include "stalled_issues_cases/LocalAndRemoteDifferentWidget.h"
#include "stalled_issues_cases/LocalAndRemoteNameConflicts.h"
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

void StalledIssuesDelegateWidgetsCache::reset()
{
    qDeleteAll(mStalledIssueHeaderWidgets);
    foreach(auto map, mStalledIssueWidgets.values())
    {
        qDeleteAll(map);
    }
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getStalledIssueHeaderWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant &issue) const
{
    auto row = index.row() % 10;

    auto header = mStalledIssueHeaderWidgets[row];

    if(!header)
    {
        header = createHeaderWidget(index, parent, issue);
        header->hide();
    }
    else
    {
        header->updateUi(index, issue);
    }

    return header;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant &issue) const
{
    auto row = index.parent().row() % 10;

    auto reason = issue.data()->getReason();
    auto& itemsByRowMap = mStalledIssueWidgets[toInt(reason)];
    auto item = itemsByRowMap[row];

    if(!item)
    {
        item = createBodyWidget(index, parent, issue);
        item->hide();

        itemsByRowMap.insert(row, item);
    }
    else
    {
        item->updateUi(index, issue);
    }

    return item;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueHeaderWidget(const QModelIndex& index, QWidget* parent, const StalledIssueVariant &issue) const
{
    return createHeaderWidget(index, parent, issue);
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant& issue) const
{
   return createBodyWidget(index, parent, issue);
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::createBodyWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant &issue) const
{
    StalledIssueBaseDelegateWidget* item(nullptr);
    auto reason = issue.data()->getReason();

    switch(reason)
    {
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        {
            item = new LocalAndRemoteDifferentWidget(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced:
        {
            item = new LocalAndRemoteNameConflicts(parent);
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
        item->updateUi(index, issue);
    }

    return item;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::createHeaderWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant &issue) const
{
    StalledIssueHeader* header(nullptr);

    switch(issue.data()->getReason())
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
        case mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced:
        {
            header  = new NameConflictsHeader(parent);
            break;
        }
        default:
        {
            header = new DefaultHeader(parent);
        }
    }

    if(header)
    {
        header->updateUi(index, issue);
    }

    return header;
}
