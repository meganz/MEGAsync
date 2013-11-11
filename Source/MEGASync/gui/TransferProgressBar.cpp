#include "TransferProgressBar.h"
#include "ui_TransferProgressBar.h"

TransferProgressBar::TransferProgressBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferProgressBar)
{
    ui->setupUi(this);
}

TransferProgressBar::~TransferProgressBar()
{
    delete ui;
}

void TransferProgressBar::setProgress(int value)
{
    if(value>100) value = 100;
    this->progress = value;
    ui->wProgress->resize(value*3.6, ui->wProgress->width());
}
