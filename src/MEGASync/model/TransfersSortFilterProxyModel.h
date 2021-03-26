#ifndef TRANSFERSSORTFILTERPROXYMODEL_H
#define TRANSFERSSORTFILTERPROXYMODEL_H

#include "TransferItem2.h"

#include <QSortFilterProxyModel>

class TransfersSortFilterProxyModel : public QSortFilterProxyModel
{
        Q_OBJECT

    public:
        TransfersSortFilterProxyModel(QObject *parent = 0);

        bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                      const QModelIndex &destinationParent, int destinationChild);

        void setTransferType(const QSet<int> transferTypes);
        void addTransferType(const QSet<int> transferTypes);
        void setTransferState(const QSet<int> transferStates);
        void addTransferState(const QSet<int> transferStates);
        void setFileType(const QSet<TransferData::FileTypes> fileTypes);
        void addFileType(const QSet<TransferData::FileTypes> fileTypes);
        void resetAllFilters();

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;


    private:
        QSet<int> mTransferType;
        QSet<int> mTransferState;
        QSet<TransferData::FileTypes> mFileType;
};

#endif // TRANSFERSSORTFILTERPROXYMODEL_H
