#include "GuestWidget.h"
#include "ui_GuestWidget.h"

GuestWidget::GuestWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GuestWidget)
{
    ui->setupUi(this);
}

GuestWidget::~GuestWidget()
{
    delete ui;
}
