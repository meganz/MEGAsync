#ifndef STALLEDISSUEHEADERWIDGETMANAGER_H
#define STALLEDISSUEHEADERWIDGETMANAGER_H

#include "StalledIssue.h"
#include "megaapi.h"

#include <QMap>
#include <QModelIndex>
#include <QPointer>
#include <QStyledItemDelegate>

class StalledIssueHeader;
class StalledIssueHeaderCase;
class StalledIssueFilePath;
class StalledIssuesProxyModel;
class StalledIssueBaseDelegateWidget;

class StalledIssuesDelegateWidgetsCache
{
public:
    static const int DELEGATEWIDGETS_CACHESIZE;

    StalledIssuesDelegateWidgetsCache(QStyledItemDelegate* delegate);

    StalledIssueHeader* getStalledIssueHeaderWidget(const QModelIndex& index, QWidget *parent, const StalledIssueVariant &issue, const QSize& size) const;
    StalledIssueBaseDelegateWidget* getStalledIssueInfoWidget(const QModelIndex& index, QWidget *parent, const StalledIssueVariant &issue, const QSize& size) const;

    static bool adaptativeHeight(mega::MegaSyncStall::SyncStallReason reason);

    void setProxyModel(StalledIssuesProxyModel *proxyModel);

    void reset();

private:
    mutable QMap<int,QPointer<StalledIssueHeader>> mStalledIssueHeaderWidgets;
    mutable QMap<int, QMap<int, QPointer<StalledIssueBaseDelegateWidget>>> mStalledIssueWidgets;

    int getMaxCacheRow(int row) const;

    StalledIssueBaseDelegateWidget* createBodyWidget(const QModelIndex& index, QWidget *parent, const StalledIssueVariant &issue) const;
    StalledIssueHeaderCase* createHeaderCaseWidget(StalledIssueHeader* header, const StalledIssueVariant &issue) const;

    StalledIssuesProxyModel* mProxyModel;
    QStyledItemDelegate* mDelegate;
};

#endif // STALLEDISSUEHEADERWIDGETMANAGER_H
