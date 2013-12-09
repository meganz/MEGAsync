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

void TransferProgressBar::setProgress(int permil)
{
	if(permil>1000) permil = 1000;
	ui->wProgress->resize(permil*0.36, ui->wProgress->width());
}
