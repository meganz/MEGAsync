#ifndef STALLEDISSUEDELEGATE_H
#define STALLEDISSUEDELEGATE_H

#include "StalledIssueBaseDelegateWidget.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssuesModel.h"

#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>

class StalledIssueDelegate : public QStyledItemDelegate
{
public:
    StalledIssueDelegate(StalledIssuesProxyModel* proxyModel,  QAbstractItemView* view);
    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&index) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    bool event(QEvent *event) override;
//    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
//    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    StalledIssueBaseDelegateWidget *getStalledIssueItemWidget(const QModelIndex &index) const;
    QAbstractItemView* mView;
    StalledIssuesProxyModel* mProxyModel;
    StalledIssuesModel* mSourceModel;
    mutable QVector<StalledIssueBaseDelegateWidget*> mStalledIssueHeaderItems;
    mutable QVector<StalledIssueBaseDelegateWidget*> mStalledIssueInfoItems;
};

#endif // STALLEDISSUEDELEGATE_H
