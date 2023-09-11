#include "IgnoresEditingDialog.h"
#include "ui_IgnoresEditingDialog.h"

IgnoresEditingDialog::IgnoresEditingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IgnoresEditingDialog)
{
    ui->setupUi(this);
}

IgnoresEditingDialog::~IgnoresEditingDialog()
{
    delete ui;
}
