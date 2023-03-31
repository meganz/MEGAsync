#include "CrashReportDialog.h"
#include "ui_CrashReportDialog.h"

CrashReportDialog::CrashReportDialog(QString crash, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CrashReportDialog)
{
    ui->setupUi(this);

    ui->cLogs->setText(ui->cLogs->text().replace(QString::fromUtf8("[Br]"), QString::fromUtf8("\n")));
    ui->tCrash->setText(crash);
    ui->bOK->setDefault(true);
}

QString CrashReportDialog::getUserMessage()
{
    return ui->tUserMessage->toPlainText();
}

bool CrashReportDialog::sendLogs()
{
    return ui->cLogs->isChecked();
}

CrashReportDialog::~CrashReportDialog()
{
    delete ui;
}
