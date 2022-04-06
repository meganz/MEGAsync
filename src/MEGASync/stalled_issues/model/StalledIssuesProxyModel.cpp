#include "StalledIssuesProxyModel.h"

#include "StalledIssueHeader.h"
#include "StalledIssueChooseWidget.h"

#include <QDebug>

StalledIssuesProxyModel::StalledIssuesProxyModel(QObject *parent) :QSortFilterProxyModel(parent)
{

}

bool StalledIssuesProxyModel::canFetchMore(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::canFetchMore(parent);
}
