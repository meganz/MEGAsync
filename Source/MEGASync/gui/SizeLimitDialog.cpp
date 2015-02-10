#include "SizeLimitDialog.h"
#include "ui_SizeLimitDialog.h"

SizeLimitDialog::SizeLimitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SizeLimitDialog)
{
    ui->setupUi(this);

    ui->eLowerThan->setMaximum(9999);
    ui->eUpperThan->setMaximum(9999);

    ui->cbExcludeUpperUnit->addItem(QString::fromUtf8("Bytes"));
    ui->cbExcludeUpperUnit->addItem(QString::fromUtf8("KB"));
    ui->cbExcludeUpperUnit->addItem(QString::fromUtf8("MB"));
    ui->cbExcludeUpperUnit->addItem(QString::fromUtf8("GB"));

    ui->cbExcludeLowerUnit->addItem(QString::fromUtf8("Bytes"));
    ui->cbExcludeLowerUnit->addItem(QString::fromUtf8("KB"));
    ui->cbExcludeLowerUnit->addItem(QString::fromUtf8("MB"));
    ui->cbExcludeLowerUnit->addItem(QString::fromUtf8("GB"));

    ui->bOK->setDefault(true);
}

bool SizeLimitDialog::upperSizeLimit()
{
    return ui->cExcludeUpperThan->isChecked();
}

bool SizeLimitDialog::lowerSizeLimit()
{
    return ui->cExcludeLowerThan->isChecked();
}

void SizeLimitDialog::setUpperSizeLimit(bool value)
{
    ui->cExcludeUpperThan->setChecked(value);
    ui->eUpperThan->setEnabled(value);
    ui->cbExcludeUpperUnit->setEnabled(value);

}
void SizeLimitDialog::setLowerSizeLimit(bool value)
{
    ui->cExcludeLowerThan->setChecked(value);
    ui->eLowerThan->setEnabled(value);
    ui->cbExcludeLowerUnit->setEnabled(value);
}

void SizeLimitDialog::setUpperSizeLimitValue(long long limit)
{
    ui->eUpperThan->setValue(limit);
}

void SizeLimitDialog::setLowerSizeLimitValue(long long limit)
{
    ui->eLowerThan->setValue(limit);
}

long long SizeLimitDialog::upperSizeLimitValue()
{
    return ui->eUpperThan->value();
}

long long SizeLimitDialog::lowerSizeLimitValue()
{
    return ui->eLowerThan->value();
}

void SizeLimitDialog::setUpperSizeLimitUnit(int unit)
{
    ui->cbExcludeUpperUnit->setCurrentIndex(unit);
}

void SizeLimitDialog::setLowerSizeLimitUnit(int unit)
{
    ui->cbExcludeLowerUnit->setCurrentIndex(unit);
}

int SizeLimitDialog::upperSizeLimitUnit()
{
    return ui->cbExcludeUpperUnit->currentIndex();
}

int SizeLimitDialog::lowerSizeLimitUnit()
{
    return ui->cbExcludeLowerUnit->currentIndex();
}

void SizeLimitDialog::on_cExcludeUpperThan_clicked()
{
    if(ui->cExcludeUpperThan->isChecked())
    {
        ui->eUpperThan->setEnabled(true);
        ui->cbExcludeUpperUnit->setEnabled(true);
    }else
    {
        ui->eUpperThan->setEnabled(false);
        ui->cbExcludeUpperUnit->setEnabled(false);
    }
}

void SizeLimitDialog::on_cExcludeLowerThan_clicked()
{
    if(ui->cExcludeLowerThan->isChecked())
    {
        ui->eLowerThan->setEnabled(true);
        ui->cbExcludeLowerUnit->setEnabled(true);
    }else
    {
        ui->eLowerThan->setEnabled(false);
        ui->cbExcludeLowerUnit->setEnabled(false);
    }
}

void SizeLimitDialog::on_bOK_clicked()
{
    if((upperSizeLimit() && !upperSizeLimitValue()) ||
            (lowerSizeLimit() && !lowerSizeLimitValue()))
    {
        QMessageBox::warning(this, tr("Warning"), tr("Limits can not be zero."), QMessageBox::Ok);
        return;
    }
    accept();
}

void SizeLimitDialog::on_bCancel_clicked()
{
    reject();
}

void SizeLimitDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

SizeLimitDialog::~SizeLimitDialog()
{
    delete ui;
}
