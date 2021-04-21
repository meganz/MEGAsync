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

        bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                      const QModelIndex &destinationParent, int destinationChild);

        void setTransferTypes(const QSet<int>& transferTypes);
        void addTransferTypes(const QSet<int>& transferTypes);
        void setTransferStates(const TransferData::TransferStates transferStates);
        void addTransferStates(const TransferData::TransferStates transferStates);
        void setFileTypes(const TransferData::FileTypes fileTypes);
        void addFileTypes(const TransferData::FileTypes fileTypes);
        void resetAllFilters();
        void setSortBy(SORT_BY sortCriterion);

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;


    private:
        QSet<int> mTransferTypes;
        TransferData::TransferStates mTransferStates;
        TransferData::FileTypes mFileTypes;
        SORT_BY mSortCriterion;
};

#endif // TRANSFERSSORTFILTERPROXYMODEL_H
