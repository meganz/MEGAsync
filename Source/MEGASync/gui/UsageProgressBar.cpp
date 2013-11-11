#include "UsageProgressBar.h"
#include "ui_UsageProgressBar.h"

UsageProgressBar::UsageProgressBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UsageProgressBar)
{
    ui->setupUi(this);
}

UsageProgressBar::~UsageProgressBar()
{
    delete ui;
}

void UsageProgressBar::setProgress(int value)
{
    if(value>100) value = 100;
    this->progress = value;
    ui->wProgress->resize(value*3.6, ui->wProgress->width());
    if(value<80)
        ui->wProgress->setStyleSheet("background-color: green;");
    else
        ui->wProgress->setStyleSheet("background-color: red;");
}
