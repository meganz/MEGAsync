#include "BindFolderDialog.h"
#include "ui_BindFolderDialog.h"

BindFolderDialog::BindFolderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog)
{
    ui->setupUi(this);
}

BindFolderDialog::~BindFolderDialog()
{
    delete ui;
}

long long BindFolderDialog::getMegaFolder()
{
    return ui->wBinder->selectedMegaFolder();
}

QString BindFolderDialog::getLocalFolder()
{
    return ui->wBinder->selectedLocalFolder();
}
