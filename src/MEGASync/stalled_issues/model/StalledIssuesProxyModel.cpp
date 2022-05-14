#include "StalledIssuesProxyModel.h"

#include "StalledIssueHeader.h"
#include "StalledIssueChooseWidget.h"
#include "StalledIssuesModel.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

StalledIssuesProxyModel::StalledIssuesProxyModel(QObject *parent) :QSortFilterProxyModel(parent)
{}

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

    auto sourceM = qobject_cast<StalledIssuesModel*>(sourceModel());
    if(sourceM->rowCount(QModelIndex()) != 0)
    {
        emit uiBlocked();

        //Test if it is worth it, because there is not sorting and the sort takes longer than filtering.
        QFuture<void> filtered = QtConcurrent::run([this, sourceM](){
            sourceM->lockModelMutex(true);
            sourceM->blockSignals(true);
            blockSignals(true);
            invalidateFilter();
            sourceM->lockModelMutex(false);
            sourceM->blockSignals(false);
            blockSignals(false);
            emit uiUnblocked();
        });
    }
}

void StalledIssuesProxyModel::updateFilter()
{
    filter(mFilterCriterion);
}

void StalledIssuesProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
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
        else
        {
            auto filterCriterion = StalledIssue::getCriterionByReason(d.getReason());
            return filterCriterion == mFilterCriterion;
        }
    }
    else
    {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
}
