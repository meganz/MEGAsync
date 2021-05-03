#ifndef TRANSFERSSORTFILTERPROXYMODEL_H
#define TRANSFERSSORTFILTERPROXYMODEL_H

#include "TransferItem2.h"

#include <QSortFilterProxyModel>

class TransfersSortFilterProxyModel : public QSortFilterProxyModel
{
        Q_OBJECT

    public:

        enum SORT_BY
        {
            PRIORITY   = 0,
            TOTAL_SIZE = 1,
            NAME       = 2,
        };

        TransfersSortFilterProxyModel(QObject *parent = 0);
        ~TransfersSortFilterProxyModel();

        bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                      const QModelIndex& destinationParent, int destinationChild);

        void setTransferTypes(TransferData::TransferTypes transferTypes);
        void addTransferTypes(TransferData::TransferTypes transferTypes);
        void setTransferStates(TransferData::TransferStates transferStates);
        void addTransferStates(TransferData::TransferStates transferStates);
        void setFileTypes(TransferData::FileTypes fileTypes);
        void addFileTypes(TransferData::FileTypes fileTypes);
        void resetAllFilters();
        void setSortBy(SORT_BY sortCriterion);
        int  getNumberOfItems(TransferData::TransferType transferType);
        void resetNumberOfItems();

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    private:
        TransferData::TransferStates mTransferStates;
        TransferData::TransferTypes mTransferTypes;
        TransferData::FileTypes mFileTypes;
        SORT_BY mSortCriterion;
        int* mDlNumber;
        int* mUlNumber;
};

#endif // TRANSFERSSORTFILTERPROXYMODEL_H
