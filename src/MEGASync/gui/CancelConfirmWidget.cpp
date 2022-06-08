#include "CancelConfirmWidget.h"
#include "ui_CancelConfirmWidget.h"

#include "BlurredShadowEffect.h"

CancelConfirmWidget::CancelConfirmWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CancelConfirmWidget)
{
    ui->setupUi(this);
    ui->pDismiss->setGraphicsEffect(CreateBlurredShadowEffect());
    ui->pProceed->setGraphicsEffect(CreateBlurredShadowEffect(QColor(217, 0, 7, 76)));

    connect(ui->pDismiss, &QPushButton::clicked, this, &CancelConfirmWidget::onDismissClicked);
    connect(ui->pProceed, &QPushButton::clicked, this, &CancelConfirmWidget::onProceedClicked);
}

CancelConfirmWidget::~CancelConfirmWidget()
{
    delete ui;
}

void CancelConfirmWidget::show()
{
    enableButtons(true);
}

void CancelConfirmWidget::onDismissClicked()
{
    emit dismiss();
}

void CancelConfirmWidget::onProceedClicked()
{
    enableButtons(false);
    emit proceed();
}

void CancelConfirmWidget::enableButtons(bool value)
{
    ui->pDismiss->setEnabled(value);
    ui->pProceed->setEnabled(value);
}
