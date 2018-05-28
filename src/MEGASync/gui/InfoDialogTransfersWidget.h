#ifndef INFODIALOGTRANSFERSWIDGET_H
#define INFODIALOGTRANSFERSWIDGET_H

#include <QWidget>
#include "QTransfersModel.h"
#include "QCustomTransfersModel.h"
#include "MegaTransferDelegate.h"

namespace Ui {
class InfoDialogTransfersWidget;
}

class InfoDialogTransfersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoDialogTransfersWidget(QWidget *parent = 0);
    void setupTransfers();
    QCustomTransfersModel *getModel();
    ~InfoDialogTransfersWidget();

private:
    Ui::InfoDialogTransfersWidget *ui;
    QCustomTransfersModel *model;
    MegaTransferDelegate *tDelegate;

private:
    void configureTransferView();

private slots:
    void enableAlternateRowStyle();
    void disableAlternateRowStyle();
};

#endif // INFODIALOGTRANSFERSWIDGET_H
