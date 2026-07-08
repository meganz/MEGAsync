#ifndef TRANSFERSSORTFILTERPROXYBASEMODEL_H
#define TRANSFERSSORTFILTERPROXYBASEMODEL_H

#include "ILoadingViewModel.h"

#include <QSortFilterProxyModel>

class TransferBaseDelegateWidget;
class TransfersModel;

// isWorking() is inherited pure from ILoadingViewModel: every transfers proxy attached to a
// loading-scene view must state explicitly whether it does threaded work.
class TransfersSortFilterProxyBaseModel: public QSortFilterProxyModel, public ILoadingViewModel
{
    Q_OBJECT

public:
    TransfersSortFilterProxyBaseModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent)
    {}
    ~TransfersSortFilterProxyBaseModel(){}

    virtual TransferBaseDelegateWidget* createTransferManagerItem(QWidget *parent) = 0;

protected:
    int columnCount(const QModelIndex &) const override {return 1;}
};

#endif // TRANSFERSSORTFILTERPROXYBASEMODEL_H
