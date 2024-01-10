#include "StalledIssuesDelegateWidgetsCache.h"

#include "stalled_issues_cases/LocalAndRemoteDifferentWidget.h"
#include "stalled_issues_cases/LocalAndRemoteNameConflicts.h"
#include "stalled_issues_cases/OtherSideMissingOrBlocked.h"
#include "stalled_issues_cases/StalledIssuesCaseHeaders.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueFilePath.h"

#include "Utilities.h"

const int StalledIssuesDelegateWidgetsCache::DELEGATEWIDGETS_CACHESIZE = 30;

StalledIssuesDelegateWidgetsCache::StalledIssuesDelegateWidgetsCache(QStyledItemDelegate *delegate)
    : mDelegate(delegate)
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

    mStalledIssueWidgets.clear();
}

int StalledIssuesDelegateWidgetsCache::getMaxCacheRow(int row) const
{
    auto nbRowsMaxInView(DELEGATEWIDGETS_CACHESIZE);
    return row % nbRowsMaxInView;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getStalledIssueHeaderWidget(const QModelIndex &sourceIndex,
                                                                                   const QModelIndex& proxyIndex,
                                                                                   QWidget *parent,
                                                                                   const StalledIssueVariant &issue,
                                                                                   const QSize &size) const
{
    auto row = getMaxCacheRow(proxyIndex.row());
    auto& header = mStalledIssueHeaderWidgets[row];

    bool isNew(!header);
    bool needsUpdate(isNew ||
               header->getCurrentIndex() != sourceIndex ||
               issue.consultData()->needsUIUpdate(StalledIssue::Type::Header));

    if(isNew)
    {
        header = new StalledIssueHeader(parent);
        header->setDelegate(mDelegate);
    }

    createHeaderCaseWidget(header, issue);

    if(isNew)
    {
        header->resize(QSize(size.width(), header->size().height()));
    }

    if(needsUpdate)
    {
        header->updateUi(sourceIndex, issue);
    }

    if(isNew)
    {
        header->show();
        header->hide();
    }

    if(needsUpdate)
    {
        header->refreshCaseActions();
        header->refreshCaseTitles();
    }

    return header;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getStalledIssueInfoWidget(const QModelIndex& sourceIndex,
                                                                                             const QModelIndex& proxyIndex,
                                                                                             QWidget *parent,
                                                                                             const StalledIssueVariant &issue,
                                                                                             const QSize& size) const
{
    auto row = getMaxCacheRow(proxyIndex.parent().row());

    auto reason = issue.consultData()->getReason();
    auto& itemsByRowMap = mStalledIssueWidgets[toInt(reason)];
    auto& item = itemsByRowMap[row];

    bool needsUpdate(!item ||
               item->getCurrentIndex() != sourceIndex);

    if(needsUpdate)
    {
        if(item)
        {
            item->deleteLater();
        }

        item = createBodyWidget(parent, issue);
        item->resize(QSize(size.width(), item->size().height()));
        item->show();
        item->hide();
        item->updateUi(sourceIndex, issue);
        item->setDelegate(mDelegate);

        itemsByRowMap.insert(row, item);
    }
    else if(needsUpdate || issue.consultData()->needsUIUpdate(StalledIssue::Type::Body))
    {
        item->updateUi(sourceIndex, issue);
    }

    return item;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::createBodyWidget(QWidget *parent, const StalledIssueVariant &issue) const
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
            if(issue.consultData()->isSymLink())
            {
                headerCase = new SymLinkHeader(header);
            }
            else if(issue.consultData()->isUndecrypted())
            {
                headerCase = new CloudNodeUndecryptedHeader(header);
            }
            else
            {
                headerCase = new FileIssueHeader(header);
            }
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
            if(issue.consultData()->missingFingerprint())
            {
                headerCase = new CloudFingerprintMissingHeader(header);
            }
            else
            {
                headerCase = new DownloadIssueHeader(header);
            }
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
