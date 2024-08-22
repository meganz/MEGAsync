#include "TransfersAccountInfoWidget.h"

#include "ui_TransfersAccountInfoWidget.h"

TransfersAccountInfoWidget::TransfersAccountInfoWidget(QWidget* parent):
    QWidget(parent),
    ui(new Ui::TransfersAccountInfoWidget)
{
    ui->setupUi(this);
}

TransfersAccountInfoWidget::~TransfersAccountInfoWidget()
{
    delete ui;
}
