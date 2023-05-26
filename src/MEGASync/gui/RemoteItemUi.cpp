#include "RemoteItemUi.h"
#include "ui_RemoteItemUi.h"

RemoteItemUi::RemoteItemUi(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::RemoteItemUi)
{
    ui->setupUi(this);
}

RemoteItemUi::~RemoteItemUi()
{
    delete ui;
}

void RemoteItemUi::setUsePermissions(const bool use)
{
    if (!use)
    {
        ui->bPermissions->hide();
    }
}
