#include "StalledIssuesDelegateWidgetsCache.h"

#include "stalled_issues_cases/LocalAndRemoteDifferentWidget.h"
#include "stalled_issues_cases/StalledIssuesCaseHeaders.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueFilePath.h"

#include "Utilities.h"


StalledIssuesDelegateWidgetsCache::StalledIssuesDelegateWidgetsCache()
{
}

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
        default:
        {
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

        }
    }

    if(header)
    {
        header->updateUi(index, data);
    }

    return header;
}
