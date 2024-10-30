#include "ProgressIndicatorDialog.h"

#include "ui_ProgressIndicatorDialog.h"

ProgressIndicatorDialog::ProgressIndicatorDialog(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ProgressIndicatorDialog)
{
    ui->setupUi(this);
}

ProgressIndicatorDialog::~ProgressIndicatorDialog()
{
    delete ui;
}
