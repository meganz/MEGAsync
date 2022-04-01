#include "StalledIssuesDialog.h"
#include "ui_StalledIssuesDialog.h"

StalledIssuesDialog::StalledIssuesDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssuesDialog)
{
    ui->setupUi(this);
}

StalledIssuesDialog::~StalledIssuesDialog()
{
    delete ui;
}
