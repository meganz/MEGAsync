#include "CrashReportDialog.h"
#include "ui_CrashReportDialog.h"

CrashReportDialog::CrashReportDialog(QString crash, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CrashReportDialog)
{
    ui->setupUi(this);
    ui->tCrash->setText(crash);
}

QString CrashReportDialog::getUserMessage()
{
    return ui->tUserMessage->toPlainText();
}

CrashReportDialog::~CrashReportDialog()
{
    delete ui;
}
