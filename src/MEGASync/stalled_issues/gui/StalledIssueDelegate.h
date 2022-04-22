#ifndef STALLEDISSUEDELEGATE_H
#define STALLEDISSUEDELEGATE_H

#include "StalledIssuesProxyModel.h"
#include "StalledIssuesModel.h"
#include "StalledIssuesDelegateWidgetsCache.h"

#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QPointer>

class StalledIssueBaseDelegateWidget;
class StalledIssuesView;

class StalledIssueDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    StalledIssueDelegate(StalledIssuesProxyModel* proxyModel,  StalledIssuesView* view);
    ~StalledIssueDelegate();
    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&index) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

protected slots:
    void onHoverEnter(const QModelIndex& index);
    void onHoverLeave(const QModelIndex&index);

private slots:
    void onIssueFixed();

private:
    StalledIssueBaseDelegateWidget *getStalledIssueItemWidget(const QModelIndex &index, const StalledIssue &data) const;
    StalledIssueBaseDelegateWidget *getNonCacheStalledIssueItemWidget(const QModelIndex &index, QWidget *parent, const StalledIssue& data) const;

    StalledIssuesView* mView;
    StalledIssuesProxyModel* mProxyModel;
    StalledIssuesModel* mSourceModel;
    StalledIssuesDelegateWidgetsCache mCacheManager;

    mutable QPointer<StalledIssueBaseDelegateWidget> mEditor;
};

#endif // STALLEDISSUEDELEGATE_H
