#include "CancelConfirmWidget.h"
#include "ui_cancelconfirmwidget.h"

CancelConfirmWidget::CancelConfirmWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CancelConfirmWidget)
{
    ui->setupUi(this);
}

CancelConfirmWidget::~CancelConfirmWidget()
{
    delete ui;
}

void CancelConfirmWidget::on_pDismiss_clicked()
{
    emit dismiss();
}

void CancelConfirmWidget::on_pProceed_clicked()
{
    emit proceed();
}
