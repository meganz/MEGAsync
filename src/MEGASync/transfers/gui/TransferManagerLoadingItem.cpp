#include "TransferManagerLoadingItem.h"
#include "ui_TransferManagerLoadingItem.h"

#include <MegaTransferView.h>

#include <QScrollBar>

///LOADING ITEM -> TO RENDER ON DELEGATE
TransferManagerLoadingItem::TransferManagerLoadingItem(QAbstractItemView *parent) :
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
