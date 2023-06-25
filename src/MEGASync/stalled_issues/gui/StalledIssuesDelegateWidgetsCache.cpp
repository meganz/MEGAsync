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
    foreach(auto headerCase, mStalledIssueHeaderWidgets)
    {
        headerCase->reset();
    }

    mStalledIssueHeaderWidgets.clear();

    foreach(auto map, mStalledIssueWidgets.values())
    {
        qDeleteAll(map);
    }
}

int StalledIssuesDelegateWidgetsCache::getMaxCacheRow(int row) const
{
    auto nbRowsMaxInView(30);
    return row % nbRowsMaxInView;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getStalledIssueHeaderWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant &issue, bool isEditor) const
{
    auto row = getMaxCacheRow(index.row());
    auto& header = mStalledIssueHeaderWidgets[row];

    if(!header)
    {
        header = new StalledIssueHeader(parent);
        header->hide();
    }
    //We don´t need to update it if is an editor and exists
    else if(isEditor)
    {
        return header;
    }

    createHeaderCaseWidget(header, issue);
    header->updateUi(index, issue);

    return header;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant &issue, bool isEditor) const
{
    auto row = getMaxCacheRow(index.parent().row());

    auto reason = issue.consultData()->getReason();
    auto& itemsByRowMap = mStalledIssueWidgets[toInt(reason)];
    auto& item = itemsByRowMap[row];

    //We don´t need to update it if is an editor and exists
    if(item && isEditor)
    {
        return item;
    }

    if(item && item->getData().consultData()->getReason() == issue.consultData()->getReason())
    {
        item->updateUi(index, issue);
    }
    else if(!item || (item && item->getData().consultData()->getReason() != issue.consultData()->getReason()))
    {
        if(item)
        {
            item->deleteLater();
        }

        item = createBodyWidget(index, parent, issue);
        item->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
        item->setAttribute(Qt::WA_WState_Hidden , true);

        itemsByRowMap.insert(row, item);
    }

    return item;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::createBodyWidget(const QModelIndex &index, QWidget *parent, const StalledIssueVariant &issue) const
{
    StalledIssueBaseDelegateWidget* item(nullptr);
    auto reason = issue.consultData()->getReason();

    switch(reason)
    {
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        {
            item = new LocalAndRemoteDifferentWidget(issue.consultData()->getOriginalStall(), parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced:
        {
            item = new LocalAndRemoteNameConflicts(issue.consultData()->getOriginalStall(), parent);
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

bool StalledIssuesDelegateWidgetsCache::adaptativeHeight(mega::MegaSyncStall::SyncStallReason reason)
{
    switch(reason)
    {
        case mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced:
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        {
            return true;
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
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        default:
            return false;
    }
}

StalledIssueHeaderCase* StalledIssuesDelegateWidgetsCache::createHeaderCaseWidget(StalledIssueHeader* header, const StalledIssueVariant &issue) const
{
    QPointer<StalledIssueHeaderCase> headerCase(nullptr);

    switch(issue.consultData()->getReason())
    {
        case mega::MegaSyncStall::SyncStallReason::FileIssue:
        {
            headerCase = new FileIssueHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur:
        {
            headerCase = new MoveOrRenameCannotOccurHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning:
        {
            headerCase = new DeleteOrMoveWaitingOnScanningHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        {
            headerCase = new DeleteWaitingOnMovesHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::UploadIssue:
        {
            headerCase = new UploadIssueHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::DownloadIssue:
        {
            headerCase = new DownloadIssueHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CannotCreateFolder:
        {
            headerCase = new CannotCreateFolderHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CannotPerformDeletion:
        {
            headerCase = new CannotPerformDeletionHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::FolderMatchedAgainstFile:
        {
            headerCase = new FolderMatchedAgainstFileHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        {
            headerCase = new SyncItemExceedsSupoortedTreeDepthHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        {
            headerCase = new LocalAndRemoteChangedSinceLastSyncedStateHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        {
            headerCase = new LocalAndRemotePreviouslyUnsyncedDifferHeader(header);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced:
        {
            headerCase = new NameConflictsHeader(header);
            break;
        }
        default:
        {
            headerCase = new DefaultHeader(header);
        }
    }

    return headerCase.data();
}
