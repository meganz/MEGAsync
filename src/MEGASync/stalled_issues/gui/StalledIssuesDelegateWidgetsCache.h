#ifndef STALLEDISSUEHEADERWIDGETMANAGER_H
#define STALLEDISSUEHEADERWIDGETMANAGER_H

#include "StalledIssue.h"
#include "megaapi.h"

#include <QMap>
#include <QModelIndex>

class StalledIssueHeader;
class StalledIssueFilePath;
class StalledIssuesProxyModel;
class StalledIssueBaseDelegateWidget;

class StalledIssuesDelegateWidgetsCache
{
public:
    StalledIssuesDelegateWidgetsCache();

    StalledIssueHeader* getStalledIssueHeaderWidget(const QModelIndex& index, QWidget *parent, StalledIssueDataPtr data) const;
    StalledIssueBaseDelegateWidget* getStalledIssueInfoWidget(const QModelIndex& index, QWidget *parent, StalledIssueDataPtr data) const;

    StalledIssueHeader* getNonCacheStalledIssueHeaderWidget(const QModelIndex &index, QWidget *parent, StalledIssueDataPtr data) const;
    StalledIssueBaseDelegateWidget* getNonCacheStalledIssueInfoWidget(const QModelIndex& index, QWidget *parent, StalledIssueDataPtr data) const;

    void setProxyModel(StalledIssuesProxyModel *proxyModel);

private:
    mutable QMap<int, StalledIssueHeader*> mStalledIssueHeaderWidgets;
    mutable QMap<int, QMap<int, StalledIssueBaseDelegateWidget*>> mStalledIssueWidgets;

    StalledIssuesProxyModel* mProxyModel;
};

#endif // STALLEDISSUEHEADERWIDGETMANAGER_H
