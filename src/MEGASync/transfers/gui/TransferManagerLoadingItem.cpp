#include "TransferManagerLoadingItem.h"
#include "ui_TransferManagerLoadingItem.h"

///LOADING ITEM -> TO RENDER ON DELEGATE
TransferManagerLoadingItem::TransferManagerLoadingItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferManagerLoadingItem)
{
    ui->setupUi(this);
}

TransferManagerLoadingItem::~TransferManagerLoadingItem()
{
    delete ui;
}

QSize TransferManagerLoadingItem::widgetSize()
{
    return QSize(772, 64);
}
