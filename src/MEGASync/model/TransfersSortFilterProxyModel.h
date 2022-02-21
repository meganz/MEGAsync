#ifndef TRANSFERSSORTFILTERPROXYMODEL_H
#define TRANSFERSSORTFILTERPROXYMODEL_H

#include "TransferItem.h"

#include <QSortFilterProxyModel>
#include <QReadWriteLock>
#include <QFutureWatcher>

class TransferBaseDelegateWidget;
class QTransfersModel;

class TransfersSortFilterProxyModel : public QSortFilterProxyModel
{
        Q_OBJECT

public:
        TransfersSortFilterProxyModel(QObject *parent = 0);
        ~TransfersSortFilterProxyModel();

        bool moveRows(const QModelIndex& proxyParent, int proxyRow, int count,
                      const QModelIndex& destinationParent, int destinationChild) override;
        void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
        void invalidate();
        void setSourceModel(QAbstractItemModel *sourceModel) override;
        void setFilterFixedString(const QString &pattern);
        void setFilters(const TransferData::TransferTypes transferTypes,
                        const TransferData::TransferStates transferStates,
                        const TransferData::FileTypes fileTypes);
        void updateFilters();
        void resetAllFilters(bool invalidate = false);
        int  getNumberOfItems(TransferData::TransferType transferType);
        //void refreshNumberOfItems();
        void resetNumberOfItems();

        virtual TransferBaseDelegateWidget* createTransferManagerItem(QWidget *parent);

        int getPausedTransfers() const;
        bool isAnyPaused() const;

signals:
        void modelAboutToBeChanged();
        void modelChanged();
        void searchNumbersChanged();
        void modelAboutToBeSorted();
        void modelSorted();
        void transferPauseResume(bool);

protected slots:
        void onCancelClearTransfer();
        void onPauseResumeTransfer();
        void onRetryTransfer();

protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
        int columnCount(const QModelIndex &parent) const override;

protected:
        TransferData::TransferStates mTransferStates;
        TransferData::TransferTypes mTransferTypes;
        TransferData::FileTypes mFileTypes;
        TransferData::TransferStates mNextTransferStates;
        TransferData::TransferTypes mNextTransferTypes;
        TransferData::FileTypes mNextFileTypes;
        SortCriterion mSortCriterion;
        mutable QList<int> mDlNumber;
        mutable QList<int> mUlNumber;
        QMutex* mFilterMutex;
        QMutex* mActivityMutex;
        bool mSearchCountersOn;

private slots:
        void onRowsRemoved(const QModelIndex& parent, int first, int last);
        void onRowsAboutToBeInserted(const QModelIndex&, int, int);
        void onRowsInserted(const QModelIndex&, int, int);
        void onModelSortedFiltered();

private:
        ThreadPool* mThreadPool;
        QFutureWatcher<void> mFilterWatcher;
        QString mFilterText;
};

Q_DECLARE_METATYPE(QAbstractItemModel::LayoutChangeHint)

#endif // TRANSFERSSORTFILTERPROXYMODEL_H
