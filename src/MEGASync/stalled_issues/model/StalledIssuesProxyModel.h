#ifndef STALLEDISSUESPROXYMODEL_H
#define STALLEDISSUESPROXYMODEL_H

#include <QSortFilterProxyModel>

class StalledIssueBaseDelegateWidget;

class StalledIssuesProxyModel : public QSortFilterProxyModel
{
public:
    StalledIssuesProxyModel(QObject *parent = nullptr);

    StalledIssueBaseDelegateWidget *createTransferManagerItem(const QModelIndex& index,
                                                              QWidget *parent);

};

#endif // STALLEDISSUESPROXYMODEL_H
