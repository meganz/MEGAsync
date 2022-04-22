#ifndef STALLEDISSUESPROXYMODEL_H
#define STALLEDISSUESPROXYMODEL_H

#include "StalledIssue.h"

#include <QSortFilterProxyModel>

class StalledIssueBaseDelegateWidget;

class StalledIssuesProxyModel : public QSortFilterProxyModel
{
public:
    StalledIssuesProxyModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;

    void filter(StalledIssueFilterCriterion filterCriterion);

protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    StalledIssueFilterCriterion mFilterCriterion;

};

#endif // STALLEDISSUESPROXYMODEL_H
