#include "InfoDialogTransferLoadingItem.h"
#include "ui_InfoDialogTransferLoadingItem.h"

#include <InfoDialogTransferDelegateWidget.h>

const QRect InfoDialogTransferLoadingItem::FullRect = QRect(0,0,400,50);

InfoDialogTransferLoadingItem::InfoDialogTransferLoadingItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoDialogTransferLoadingItem)
{
    ui->setupUi(this);
}

InfoDialogTransferLoadingItem::~InfoDialogTransferLoadingItem()
{
    delete ui;
}

QSize InfoDialogTransferLoadingItem::widgetSize()
{
    return FullRect.size();
}
