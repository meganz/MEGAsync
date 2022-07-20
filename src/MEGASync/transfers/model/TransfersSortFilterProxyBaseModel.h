#ifndef TRANSFERSSORTFILTERPROXYBASEMODEL_H
#define TRANSFERSSORTFILTERPROXYBASEMODEL_H

#include <QSortFilterProxyModel>

class TransferBaseDelegateWidget;
class TransfersModel;

class TransfersSortFilterProxyBaseModel : public QSortFilterProxyModel
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
