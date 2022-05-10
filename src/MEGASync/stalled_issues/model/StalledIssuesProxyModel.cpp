#include "StalledIssuesProxyModel.h"

#include "StalledIssueHeader.h"
#include "StalledIssueChooseWidget.h"
#include "StalledIssuesModel.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

StalledIssuesProxyModel::StalledIssuesProxyModel(QObject *parent) :QSortFilterProxyModel(parent)
{
    connect(&mFilterWatcher, &QFutureWatcher<void>::finished,
            this, &StalledIssuesProxyModel::onModelFiltered);
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
    emit uiBlocked();
    mFilterCriterion = filterCriterion;

    QFuture<void> filtered = QtConcurrent::run([this](){
        auto sourceM = qobject_cast<StalledIssuesModel*>(sourceModel());
        sourceM->lockModelMutex(true);
        sourceM->blockSignals(true);
        blockSignals(true);
        invalidate();
        sourceM->lockModelMutex(false);
        sourceM->blockSignals(false);
        blockSignals(false);
    });
    mFilterWatcher.setFuture(filtered);
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
        else if(mFilterCriterion == StalledIssueFilterCriterion::NAME_CONFLICTS)
        {
            return d.isNameConflict();
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

void StalledIssuesProxyModel::onModelFiltered()
{
    emit uiUnblocked();
}
