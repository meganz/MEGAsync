#include "StalledIssuesProxyModel.h"

#include "StalledIssueHeader.h"
#include "StalledIssueChooseWidget.h"

#include <QDebug>

StalledIssuesProxyModel::StalledIssuesProxyModel(QObject *parent) :QSortFilterProxyModel(parent)
{

}

int StalledIssuesProxyModel::rowCount(const QModelIndex &parent) const
{
    if(sourceModel()->rowCount() == 0)
    {
        return 0;
    }

    return QSortFilterProxyModel::rowCount(parent);
}

void StalledIssuesProxyModel::filter(StalledIssueFilterCriterion filterCriterion)
{
    mFilterCriterion = filterCriterion;
    invalidate();
}

bool StalledIssuesProxyModel::canFetchMore(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::canFetchMore(parent);
}

bool StalledIssuesProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if(index.data().isValid())
    {
        const auto d (qvariant_cast<StalledIssue>(index.data()));
        if(mFilterCriterion == StalledIssueFilterCriterion::ALL_ISSUES)
        {
            return true;
        }
        else if(mFilterCriterion == StalledIssueFilterCriterion::NAME_CONFLICTS)
        {
            return d.isNameConflict();
        }
        else if(mFilterCriterion == StalledIssueFilterCriterion::ITEM_TYPE_CONFLICTS)
        {
            return false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
}
