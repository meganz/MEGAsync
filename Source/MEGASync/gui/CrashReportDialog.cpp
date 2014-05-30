#include "CrashReportDialog.h"
#include "ui_CrashReportDialog.h"

CrashReportDialog::CrashReportDialog(QString crash, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CrashReportDialog)
{
    ui->setupUi(this);
    ui->tCrash->setText(crash);

#ifdef __APPLE__
    ((QBoxLayout *)ui->bLayout->layout())->removeWidget(ui->bCancel);
    ((QBoxLayout *)ui->bLayout->layout())->insertWidget(0, ui->bCancel);
#endif
}

QString CrashReportDialog::getUserMessage()
{
    return ui->tUserMessage->toPlainText();
}

CrashReportDialog::~CrashReportDialog()
{
    delete ui;
}
