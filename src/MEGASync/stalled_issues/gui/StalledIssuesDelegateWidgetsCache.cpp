#include "StalledIssuesDelegateWidgetsCache.h"

#include "StalledIssueChooseWidget.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueFilePath.h"

#include "Utilities.h"

StalledIssuesDelegateWidgetsCache::StalledIssuesDelegateWidgetsCache()
{
}

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getStalledIssueHeaderWidget(const QModelIndex &index, QWidget *parent,StalledIssueDataPtr data) const
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

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent,StalledIssueDataPtr data) const
{
    auto row = index.parent().row() % 10;

    auto& itemsByRowMap = mStalledIssueWidgets[toInt(data->mReason)];
    auto item = itemsByRowMap[row];

    if(!item)
    {
        switch(data->mReason)
        {
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        {
            item = new StalledIssueChooseWidget(parent);
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

StalledIssueHeader *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueHeaderWidget(const QModelIndex& index, QWidget* parent, StalledIssueDataPtr data) const
{
    auto item  = new StalledIssueHeader(parent);
    item->updateUi(index, data);
    return item;
}

StalledIssueBaseDelegateWidget *StalledIssuesDelegateWidgetsCache::getNonCacheStalledIssueInfoWidget(const QModelIndex &index, QWidget *parent, StalledIssueDataPtr data) const
{
   StalledIssueBaseDelegateWidget* item(nullptr);

   switch(data->mReason)
   {
   case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
   {
       item = new StalledIssueChooseWidget(parent);
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
