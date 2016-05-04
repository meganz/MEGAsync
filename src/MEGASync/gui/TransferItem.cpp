#include "TransferItem.h"
#include "ui_TransferItem.h"

TransferItem::TransferItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferItem)
{
    ui->setupUi(this);
}

TransferItem::~TransferItem()
{
    delete ui;
}

