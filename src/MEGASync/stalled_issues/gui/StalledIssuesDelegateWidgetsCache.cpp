#include "StalledIssuesDelegateWidgetsCache.h"

#include "stalled_issues_cases/LocalAndRemotePreviouslyUnsynceDifferWidget.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueFilePath.h"

#include "Utilities.h"

StalledIssuesDelegateWidgetsCache::StalledIssuesDelegateWidgetsCache()
{
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getStalledIssueHeaderWidget(const QModelIndex &index, QWidget *parent, const StalledIssue &data) const
{
    auto row = index.row() % 10;

    auto item = mStalledIssueHeaderWidgets[row];

    if(!item)
    {
        item  = new StalledIssueHeader(parent);
        item->hide();
    }

    if(item)
    {
        item->updateUi(index, data);
    }

    return item;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, const StalledIssue &data) const
{
    auto row = index.parent().row() % 10;

    auto reason = data.getReason();
    auto& itemsByRowMap = mStalledIssueWidgets[toInt(reason)];
    auto item = itemsByRowMap[row];

    if(!item)
    {
        switch(reason)
        {
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        {
            item = new LocalAndRemotePreviouslyUnsynceDifferWidget(parent);
            item->hide();
            itemsByRowMap.insert(row, item);
        }
        }
    }

    if(item)
    {
        item->updateUi(index, data);
    }

    return item;
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueHeaderWidget(const QModelIndex& index, QWidget* parent, const StalledIssue &data) const
{
    auto item  = new StalledIssueHeader(parent);
    item->updateUi(index, data);
    return item;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, const StalledIssue& data) const
{
   StalledIssueBaseDelegateWidget* item(nullptr);

   auto reason = data.getReason();

   switch(reason)
   {
   case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
   {
       item = new LocalAndRemotePreviouslyUnsynceDifferWidget(parent);
   }
   }

    if(item)
    {
        item->updateUi(index, data);
    }

    return item;
}

void StalledIssuesDelegateWidgetsCache::setProxyModel(StalledIssuesProxyModel *proxyModel)
{
    mProxyModel = proxyModel;
}
