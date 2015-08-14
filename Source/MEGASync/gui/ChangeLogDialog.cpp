#include "ChangeLogDialog.h"
#include "ui_ChangeLogDialog.h"
#include <QDesktopServices>
#include <QString>
#include <QUrl>

ChangeLogDialog::ChangeLogDialog(QString version, QString SDKversion, QString changeLog, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeLogDialog)
{
    ui->setupUi(this);
    ui->tChangelog->document()->setDocumentMargin(16.0);
    ui->lVersion->setText(version);
    ui->lSDKVersion->setText(QString::fromAscii(" (") + SDKversion + QString::fromAscii(")"));
    setChangeLogNotes(changeLog);
}

ChangeLogDialog::~ChangeLogDialog()
{
    delete ui;
}

void ChangeLogDialog::setChangeLogNotes(QString notes)
{
    ui->tChangelog->setHtml(QString::fromUtf8("<p style=\"line-height: 119%;\"><span style=\"margin: 16px; font-family: Helvetica; font-size:11px; color: #333333;\">") +
                             tr("New in this version:") +
                             QString::fromUtf8("</span></p>") +
                             QString::fromUtf8("<p style=\" line-height: 146%;\"><span style=\"font-family: Helvetica; font-size:11px; color: #666666;\">") +
                             notes.replace(QString::fromUtf8("\n"), QString::fromUtf8("<br>")) +
                             QString::fromUtf8("</span></p>"));
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
