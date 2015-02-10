#include "SizeLimitDialog.h"
#include "ui_SizeLimitDialog.h"

SizeLimitDialog::SizeLimitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SizeLimitDialog)
{
    ui->setupUi(this);

    //QRegExp regExp(QString::fromUtf8("\\d+"));
    QRegExp regExp(QString::fromUtf8("^[0-9]*$"));
    ui->eUpperThan->setValidator(new QRegExpValidator(regExp, this));
    ui->eLowerThan->setValidator(new QRegExpValidator(regExp, this));

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
    if(!limit)
    {
        ui->eUpperThan->setText(QString::fromUtf8(""));
    }
    else
    {
        ui->eUpperThan->setText(QString::number(limit));
    }
}
void SizeLimitDialog::setLowerSizeLimitValue(long long limit)
{
    if(!limit)
    {
        ui->eLowerThan->setText(QString::fromUtf8(""));
    }
    else
    {
        ui->eLowerThan->setText(QString::number(limit));
    }
}
long long SizeLimitDialog::upperSizeLimitValue()
{
    bool ok;
    long long value = ui->eUpperThan->text().trimmed().toLongLong(&ok);
    return ok ? value : 0;
}
long long SizeLimitDialog::lowerSizeLimitValue()
{
    bool ok;
    long long value = ui->eLowerThan->text().trimmed().toLongLong(&ok);
    return ok ? value : 0;
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
        QMessageBox::information(this, tr("Warning"), tr("Limits can not be zero."), QMessageBox::Ok);
        return;
    }
    this->accept();
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
