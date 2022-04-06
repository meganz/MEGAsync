#ifndef STALLEDISSUESPROXYMODEL_H
#define STALLEDISSUESPROXYMODEL_H

#include <QSortFilterProxyModel>

class StalledIssueBaseDelegateWidget;

class StalledIssuesProxyModel : public QSortFilterProxyModel
{
public:
    StalledIssuesProxyModel(QObject *parent = nullptr);

protected:
    bool canFetchMore(const QModelIndex &parent) const override;

};

#endif // STALLEDISSUESPROXYMODEL_H
