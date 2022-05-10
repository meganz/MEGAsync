#ifndef STALLEDISSUESPROXYMODEL_H
#define STALLEDISSUESPROXYMODEL_H

#include "StalledIssue.h"

#include <QSortFilterProxyModel>
#include <QFutureWatcher>

class StalledIssueBaseDelegateWidget;

class StalledIssuesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    StalledIssuesProxyModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    void filter(StalledIssueFilterCriterion filterCriterion);
    void updateFilter();

    void setSourceModel(QAbstractItemModel *sourceModel) override;

signals:
    void uiBlocked();
    void uiUnblocked();

protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private slots:
    void onModelFiltered();

private:
    StalledIssueFilterCriterion mFilterCriterion;
    QFutureWatcher<void> mFilterWatcher;
};

#endif // STALLEDISSUESPROXYMODEL_H
