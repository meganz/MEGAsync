#include "SizeLimitDialog.h"
#include "ui_SizeLimitDialog.h"
#include "QMegaMessageBox.h"

SizeLimitDialog::SizeLimitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SizeLimitDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->eLowerThan->setMaximum(9999);
    ui->eUpperThan->setMaximum(9999);

    ui->cbExcludeUpperUnit->addItem(tr("Bytes"));
    ui->cbExcludeUpperUnit->addItem(tr("KB"));
    ui->cbExcludeUpperUnit->addItem(tr("MB"));
    ui->cbExcludeUpperUnit->addItem(tr("GB"));

    ui->cbExcludeLowerUnit->addItem(tr("Bytes"));
    ui->cbExcludeLowerUnit->addItem(tr("KB"));
    ui->cbExcludeLowerUnit->addItem(tr("MB"));
    ui->cbExcludeLowerUnit->addItem(tr("GB"));

    ui->bOK->setDefault(true);
    highDpiResize.init(this);
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
    // Avoid overflow. Cap to max int value.
    int lLimit(std::numeric_limits<int>::max());

    // Check if value in range. Should always be >0.
    if (limit < lLimit)
    {
        lLimit = static_cast<int>(limit);
    }

    // Set value.
    ui->eUpperThan->setValue(lLimit);
}

void SizeLimitDialog::setLowerSizeLimitValue(long long limit)
{
    // Avoid overflow. Cap to max int value.
    int lLimit(std::numeric_limits<int>::max());

    // Check if value in range. Should always be >0.
    if (limit < lLimit)
    {
        lLimit = static_cast<int>(limit);
    }

    // Set value.
    ui->eLowerThan->setValue(lLimit);
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
    if (ui->cExcludeUpperThan->isChecked())
    {
        ui->eUpperThan->setEnabled(true);
        ui->cbExcludeUpperUnit->setEnabled(true);
    }
    else
    {
        ui->eUpperThan->setEnabled(false);
        ui->cbExcludeUpperUnit->setEnabled(false);
    }
}

void SizeLimitDialog::on_cExcludeLowerThan_clicked()
{
    if (ui->cExcludeLowerThan->isChecked())
    {
        ui->eLowerThan->setEnabled(true);
        ui->cbExcludeLowerUnit->setEnabled(true);
    }
    else
    {
        ui->eLowerThan->setEnabled(false);
        ui->cbExcludeLowerUnit->setEnabled(false);
    }
}

void SizeLimitDialog::on_bOK_clicked()
{
    if ((upperSizeLimit() && !upperSizeLimitValue())
            || (lowerSizeLimit() && !lowerSizeLimitValue()))
    {
        QMegaMessageBox::warning(nullptr, tr("Warning"), tr("Size limits cannot be zero"), QMessageBox::Ok);
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
