#ifndef STALLEDISSUESPROXYMODEL_H
#define STALLEDISSUESPROXYMODEL_H

#include "ILoadingViewModel.h"
#include "StalledIssue.h"

#include <QFutureWatcher>
#include <QSortFilterProxyModel>

class StalledIssueBaseDelegateWidget;

class StalledIssuesProxyModel: public QSortFilterProxyModel, public ILoadingViewModel
{
    Q_OBJECT

public:
    StalledIssuesProxyModel(QObject *parent = nullptr);
    ~StalledIssuesProxyModel() override;

    // Cancels and waits for the concurrent filter job so it can never outlive this proxy.
    // Idempotent; called from the owner before deletion and (defense in depth) the destructor.
    void prepareForDeletion();

    int rowCount(const QModelIndex &parent) const override;
    void filter(StalledIssueFilterCriterion filterCriterion);
    bool isWorking() const override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    void updateStalledIssues();

    StalledIssueFilterCriterion filterCriterion() const;

public slots:
    void updateFilter();

signals:
    void modelFiltered();

protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private slots:
    void onModelSortedFiltered();

private:
    StalledIssueFilterCriterion mFilterCriterion;
    QFutureWatcher<void> mFilterWatcher;
    bool mTearingDown = false;
};

#endif // STALLEDISSUESPROXYMODEL_H
