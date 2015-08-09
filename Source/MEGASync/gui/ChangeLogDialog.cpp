#include "ChangeLogDialog.h"
#include "ui_ChangeLogDialog.h"
#include <QDesktopServices>

ChangeLogDialog::ChangeLogDialog(QString version, QString SDKversion, QString changeLog, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeLogDialog)
{
    ui->setupUi(this);

    ui->lVersion->setText(version);
    ui->lSDKVersion->setText(QString::fromAscii(" (") + SDKversion + QString::fromAscii(")"));
    ui->ptChangelog->setPlainText(changeLog);
}

ChangeLogDialog::~ChangeLogDialog()
{
    delete ui;
}

void ChangeLogDialog::on_bTerms_clicked()
{
    QString helpUrl = QString::fromAscii("https://mega.nz/#terms");
    QDesktopServices::openUrl(QUrl(helpUrl));
}

void ChangeLogDialog::on_bPolicy_clicked()
{
    QString helpUrl = QString::fromAscii("https://mega.nz/#privacy");
    QDesktopServices::openUrl(QUrl(helpUrl));
}
