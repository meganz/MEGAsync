#include "StalledIssuesProxyModel.h"

#include "StalledIssueHeader.h"
#include "StalledIssueChooseWidget.h"
#include "StalledIssuesModel.h"

#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

StalledIssuesProxyModel::StalledIssuesProxyModel(QObject *parent) :QSortFilterProxyModel(parent)
  , mFilterCriterion(StalledIssueFilterCriterion::ALL_ISSUES)
{
    connect(&mFilterWatcher, &QFutureWatcher<void>::finished,
            this, &StalledIssuesProxyModel::onModelSortedFiltered);
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

    auto sourceM = qobject_cast<StalledIssuesModel*>(sourceModel());
    emit layoutAboutToBeChanged();
    if(sourceM->rowCount(QModelIndex()) != 0)
    {
        qApp->processEvents();
        sourceM->blockUi();

        //Test if it is worth it, because there is not sorting and the sort takes longer than filtering.
        auto filterAction = QtConcurrent::run([this, sourceM]()
        {
            blockSignals(true);
            sourceM->blockSignals(true);

            invalidate();
            for (auto row = 0; row < rowCount(QModelIndex()); ++row)
            {
                auto proxyIndex = index(row, 0);
                hasChildren(proxyIndex);
            }
            QSortFilterProxyModel::sort(0, sortOrder());

            blockSignals(false);
            sourceM->blockSignals(false);
        });

        mFilterWatcher.setFuture(filterAction);
    }
    else
    {
        sourceM->blockUi();
        sourceM->unBlockUi();

        onModelSortedFiltered();
    }
}

void StalledIssuesProxyModel::updateFilter()
{
    filter(mFilterCriterion);
}

void StalledIssuesProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    auto sourceM = qobject_cast<StalledIssuesModel*>(sourceModel);
    if(sourceM)
    {
        connect(sourceM, &StalledIssuesModel::refreshFilter, this, &StalledIssuesProxyModel::updateFilter);
    }
}

void StalledIssuesProxyModel::updateStalledIssues()
{
    auto sourceM = qobject_cast<StalledIssuesModel*>(sourceModel());
    if(sourceM)
    {
        sourceM->updateStalledIssues();
    }
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
        const auto d (qvariant_cast<StalledIssueVariant>(index.data()));
        if(d.consultData()->isSolved() && !d.consultData()->isPotentiallySolved())
        {
            return  mFilterCriterion == StalledIssueFilterCriterion::SOLVED_CONFLICTS;
        }

        if(mFilterCriterion == StalledIssueFilterCriterion::ALL_ISSUES)
        {
            return true;
        }
        else
        {
            auto filterCriterion = StalledIssue::getCriterionByReason(d.consultData()->getReason());
            return filterCriterion == mFilterCriterion;
        }
    }
    else
    {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
}

void StalledIssuesProxyModel::onModelSortedFiltered()
{
    emit layoutChanged();
    emit modelFiltered();
}
