#include "TransfersStateInfoWidget.h"
#include "ui_TransfersStateInfoWidget.h"

TransfersStateInfoWidget::TransfersStateInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersStateInfoWidget)
{
    ui->setupUi(this);
}

TransfersStateInfoWidget::~TransfersStateInfoWidget()
{
    delete ui;
}

void TransfersStateInfoWidget::setText(const QString &text)
{
    ui->lStatus->setText(text);
}

void TransfersStateInfoWidget::setIcon(const QIcon &icon)
{
    ui->lStatusIcon->setIcon(icon);
}

void TransfersStateInfoWidget::setBackgroundStyle(const QString &text)
{
    ui->wContainer->setStyleSheet(text);
}
