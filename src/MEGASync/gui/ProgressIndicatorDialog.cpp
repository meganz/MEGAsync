#include "ProgressIndicatorDialog.h"

#include "ui_ProgressIndicatorDialog.h"

ProgressIndicatorDialog::ProgressIndicatorDialog(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ProgressIndicatorDialog)
{
    ui->setupUi(this);

    connect(ui->bCancel,
            &QPushButton::clicked,
            this,
            [this]()
            {
                emit cancelClicked();
            });
}

ProgressIndicatorDialog::~ProgressIndicatorDialog()
{
    delete ui;
}

void ProgressIndicatorDialog::resetProgressBar()
{
    ui->progressBar->reset();
}

void ProgressIndicatorDialog::setDialogTitle(const QString& text)
{
    setWindowTitle(text);
}

void ProgressIndicatorDialog::setDialogDescription(const QString& text)
{
    ui->lText->setText(text);
}

void ProgressIndicatorDialog::setMinimumProgressBarValue(int value)
{
    ui->progressBar->setMinimum(value);
}

void ProgressIndicatorDialog::setMaximumProgressBarValue(int value)
{
    ui->progressBar->setMaximum(value);
}

void ProgressIndicatorDialog::setProgressBarValue(int value)
{
    ui->progressBar->setValue(value);
}

int ProgressIndicatorDialog::getMinimumProgressBarValue()
{
    return ui->progressBar->minimum();
}

int ProgressIndicatorDialog::getMaximumProgressBarValue()
{
    return ui->progressBar->maximum();
}

int ProgressIndicatorDialog::getProgressBarValue()
{
    return ui->progressBar->value();
}
