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

void TransferProgressBar::setProgress(int permil, bool cancellable)
{
    if(permil>1000) permil = 1000;
    ui->bCancel->setVisible(cancellable);

    int newWidth = (354 * permil) / 1000;
    if(newWidth<6) newWidth = 6;
    else if(newWidth > 354-ui->bCancel->width())
        ui->bCancel->hide();
    ui->wProgress->resize(newWidth, ui->wProgress->height());
}


void TransferProgressBar::on_bCancel_clicked()
{
    emit cancel(ui->bCancel->pos().x(), ui->bCancel->pos().y());
}
