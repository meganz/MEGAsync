#include "LocalCleanScheduler.h"
#include "control/Utilities.h"
#include "ui_LocalCleanScheduler.h"

LocalCleanScheduler::LocalCleanScheduler(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocalCleanScheduler)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(windowTitle().arg(QString::fromLatin1(mega::MEGA_DEBRIS_FOLDER)));

    ui->eNumberDays->setMaximum(365);
    ui->bOK->setDefault(true);
}

bool LocalCleanScheduler::daysLimit()
{
    return ui->cRemoveFilesOlderThan->isChecked();
}

void LocalCleanScheduler::setDaysLimit(bool value)
{
    ui->cRemoveFilesOlderThan->setChecked(value);
    ui->eNumberDays->setEnabled(value);
}

void LocalCleanScheduler::setDaysLimitValue(int limit)
{
    ui->eNumberDays->setValue(limit);
}

int LocalCleanScheduler::daysLimitValue()
{
    return ui->eNumberDays->value();
}

LocalCleanScheduler::~LocalCleanScheduler()
{
    delete ui;
}

void LocalCleanScheduler::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        setWindowTitle(windowTitle().arg(QString::fromLatin1(mega::MEGA_DEBRIS_FOLDER)));
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void LocalCleanScheduler::on_cRemoveFilesOlderThan_clicked()
{
    if (ui->cRemoveFilesOlderThan->isChecked())
    {
        ui->eNumberDays->setEnabled(true);
    }
    else
    {
        ui->eNumberDays->setEnabled(false);
    }
}

void LocalCleanScheduler::on_bOK_clicked()
{
    if (daysLimit() && !daysLimitValue())
    {
        QMegaMessageBox::warning(NULL, tr("Warning"), tr("Please enter a valid value"), Utilities::getDevicePixelRatio(), QMessageBox::Ok);
        return;
    }
    accept();
}

void LocalCleanScheduler::on_bCancel_clicked()
{
    reject();
}
