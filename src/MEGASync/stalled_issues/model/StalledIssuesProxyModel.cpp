#include "StalledIssuesProxyModel.h"

#include "StalledIssueHeader.h"

StalledIssuesProxyModel::StalledIssuesProxyModel(QObject *parent) :QSortFilterProxyModel(parent)
{

}

StalledIssueBaseDelegateWidget *StalledIssuesProxyModel::createTransferManagerItem(const QModelIndex& index,
                                                                                   QWidget *parent)
{
    if(!index.parent().isValid())
    {
        auto item = new StalledIssueHeader(parent);
        return item;
    }
}
