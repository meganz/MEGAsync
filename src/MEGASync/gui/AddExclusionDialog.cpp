#include "AddExclusionDialog.h"
#include "ui_AddExclusionDialog.h"
#include "gui/MultiQFileDialog.h"
#include <QPointer>
#include "QMegaMessageBox.h"

AddExclusionDialog::AddExclusionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddExclusionDialog)
{
    ui->setupUi(this);
    ui->bOk->setDefault(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    highDpiResize.init(this);
}

AddExclusionDialog::~AddExclusionDialog()
{
    delete ui;
}

QString AddExclusionDialog::textValue()
{
    QString text = QDir::toNativeSeparators(ui->eExclusionItem->text().trimmed());

    //Remove possible trailing separators
    while (text.endsWith(QDir::separator()))
    {
        text.chop(1);
    }

    return text;
}

void AddExclusionDialog::on_bOk_clicked()
{
    QString text = textValue();
    if (text.isEmpty() || !QRegExp(text, Qt::CaseInsensitive, QRegExp::Wildcard).isValid())
    {
        QMegaMessageBox::warning(this, tr("Warning"), tr("Please enter a valid file name or absolute path."));
        return;
    }

    accept();
}

void AddExclusionDialog::on_bChoose_clicked()
{
    QPointer<AddExclusionDialog> currentDialog = this;
#ifdef __APPLE__
    QPointer<MultiQFileDialog> dialog = new MultiQFileDialog(0,  tr("Select the file or folder you want to exclude"), QDir::home().path(), false);
    dialog->setOptions(QFileDialog::DontResolveSymlinks);
    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted || dialog->selectedFiles().isEmpty())
    {
        delete dialog;
        return;
    }
    QString fPath = dialog->selectedFiles().value(0);
    delete dialog;
#else
    QString fPath = QFileDialog::getExistingDirectory(0,  tr("Select the folder you want to exclude"), QDir::home().path());
#endif

    if (!currentDialog || !fPath.size())
    {
        return;
    }

    ui->eExclusionItem->setText(QDir::toNativeSeparators(fPath));
}

#ifndef __APPLE__
void AddExclusionDialog::on_bChooseFile_clicked()
{
    QPointer<AddExclusionDialog> currentDialog = this;
    QString fPath = QFileDialog::getOpenFileName(0,  tr("Select the file you want to exclude"), QDir::home().path());
    if (!currentDialog || !fPath.size())
    {
        return;
    }

    ui->eExclusionItem->setText(QDir::toNativeSeparators(fPath));
}
#endif

void AddExclusionDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
