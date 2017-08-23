#include "AddExclusionDialog.h"
#include "ui_AddExclusionDialog.h"

AddExclusionDialog::AddExclusionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddExclusionDialog)
{
    ui->setupUi(this);
}

AddExclusionDialog::~AddExclusionDialog()
{
    delete ui;
}
