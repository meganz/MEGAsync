#ifndef INFODIALOGTRANSFERSWIDGET_H
#define INFODIALOGTRANSFERSWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>

#include "QTransfersModel2.h"
#include "TransfersSortFilterProxyModel.h"
#include "QCustomTransfersModel.h"

namespace Ui {
class InfoDialogTransfersWidget;
}

class InfoDialogCurrentTransfersProxyModel : public TransfersSortFilterProxyModel
{
    Q_OBJECT

public:
    InfoDialogCurrentTransfersProxyModel(QObject* parent);
    ~InfoDialogCurrentTransfersProxyModel();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

};

class MegaApplication;
class MegaTransferDelegate;

class InfoDialogTransfersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoDialogTransfersWidget(QWidget *parent = 0);
    void setupTransfers();
    ~InfoDialogTransfersWidget();

private:
    Ui::InfoDialogTransfersWidget *ui;
    InfoDialogCurrentTransfersProxyModel *model;
    MegaTransferDelegate *tDelegate;

private:
    void configureTransferView();

private:
    MegaApplication *app;
};

#endif // INFODIALOGTRANSFERSWIDGET_H
