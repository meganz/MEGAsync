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
        case mega::MegaSyncStall::SyncStallReason::SpecialFilesNotSupported:
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameFailed:
        case mega::MegaSyncStall::SyncStallReason::FolderContainsLockedFiles:
        case mega::MegaSyncStall::SyncStallReason::CantFingrprintFileYet:
        case mega::MegaSyncStall::SyncStallReason::CreateFolderFailed:
        case mega::MegaSyncStall::SyncStallReason::CouldNotMoveToLocalDebrisFolder:
        case mega::MegaSyncStall::SyncStallReason::UnableToLoadIgnoreFile:
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        case mega::MegaSyncStall::SyncStallReason::MoveTargetNameTooLong:
        case mega::MegaSyncStall::SyncStallReason::MatchedAgainstUnidentifiedItem:
        {
            auto filePath  = new StalledIssueFilePath(parent);
            filePath->setIndent(StalledIssueHeader::BODY_INDENT);
            item = filePath;
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        {
            item = new LocalAndRemoteDifferentWidget(parent);

            break;
        }
        case mega::MegaSyncStall::SyncStallReason::LocalFolderNotScannable:
        case mega::MegaSyncStall::SyncStallReason::ApplyMoveNeedsOtherSideParentFolderToExist:
        {
            item = new OtherSideMissingOrBlocked(parent);
            break;
        }
        default:
        {
             //Do not add a dummy body, I prefer it to break
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
        case mega::MegaSyncStall::SyncStallReason::SpecialFilesNotSupported:
        {
            header  = new SpecialFilesNotSupportedHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameFailed:
        {
            header  = new MoveOrRenameFailedHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        {
            header  = new SyncItemExceedsSupoortedTreeDepthHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::MoveTargetNameTooLong:
        {
            header  = new MoveTargetNameTooLongHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::MatchedAgainstUnidentifiedItem:
        {
            header  = new MatchedAgainstUnidentifiedItemHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::UnableToLoadIgnoreFile:
        {
            header  = new UnableToLoadIgnoreFileHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CreateFolderFailed:
        {
            header  = new CreateFolderFailedHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CouldNotMoveToLocalDebrisFolder:
        {
            header  = new CouldNotMoveToLocalDebrisFolderHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::CantFingrprintFileYet:
        {
            header  = new CantFingerprintFileYetHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::FolderContainsLockedFiles:
        {
            header  = new FolderContainsLockedFilesHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::LocalFolderNotScannable:
        {
            header  = new LocalFolderNotScannableHeader(parent);
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        {
            header  = new DeleteWaitingOnMovesHeader(parent);
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
        case mega::MegaSyncStall::SyncStallReason::ApplyMoveNeedsOtherSideParentFolderToExist:
        {
            header  = new ApplyMoveNeedsOtherSideParentFolderToExistHeader(parent);
            break;
        }
        default:
        {
            //Do not add a dummy header, I prefer it to break
        }
    }

    if(header)
    {
        header->updateUi(index, data);
    }

    return header;
}
