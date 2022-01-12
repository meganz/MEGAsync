#ifndef TRANSFERSSORTFILTERPROXYMODEL_H
#define TRANSFERSSORTFILTERPROXYMODEL_H

#include "TransferItem2.h"

#include <QSortFilterProxyModel>
#include <QReadWriteLock>

class TransfersSortFilterProxyModel : public QSortFilterProxyModel
{
        Q_OBJECT

    public:

        enum SortCriterion
        {
            PRIORITY   = 0,
            TOTAL_SIZE = 1,
            NAME       = 2,
        };

        TransfersSortFilterProxyModel(QObject *parent = 0);
        ~TransfersSortFilterProxyModel();

        bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                      const QModelIndex& destinationParent, int destinationChild) override;
        void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
        void setFilterFixedString(const QString &pattern);
        void setFilters(const TransferData::TransferTypes transferTypes,
                        const TransferData::TransferStates transferStates,
                        const TransferData::FileTypes fileTypes);
        void applyFilters(bool invalidate = true);
        void resetAllFilters(bool invalidate = false);
        void setSortBy(SortCriterion sortBy);
        int  getNumberOfItems(TransferData::TransferType transferType);
        void resetNumberOfItems();

    signals:
        void modelAboutToBeFiltered();
        void modelFiltered();
        void modelAboutToBeSorted();
        void modelSorted();

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    protected:
        TransferData::TransferStates mTransferStates;
        TransferData::TransferTypes mTransferTypes;
        TransferData::FileTypes mFileTypes;
        TransferData::TransferStates mNextTransferStates;
        TransferData::TransferTypes mNextTransferTypes;
        TransferData::FileTypes mNextFileTypes;
        SortCriterion mSortCriterion;
        SortCriterion mNextSortCriterion;
        int* mDlNumber;
        int* mUlNumber;
        QMutex* mFilterMutex;
        QMutex* mActivityMutex;
};

#endif // TRANSFERSSORTFILTERPROXYMODEL_H
