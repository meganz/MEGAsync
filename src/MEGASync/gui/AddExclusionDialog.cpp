#include "AddExclusionDialog.h"
#include "ui_AddExclusionDialog.h"
#include "gui/MultiQFileDialog.h"
#include <QPointer>
#include <QMessageBox>

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

QString AddExclusionDialog::textValue()
{
    return ui->eExclusionItem->text();
}

void AddExclusionDialog::on_bOk_clicked()
{
    QString text = ui->eExclusionItem->text();
    text = text.trimmed();
    if (text.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a valid file name, path or expression."));
        return;
    }

    if (!text.contains(QDir::separator())) // Not candidate local path, check if regexp is valid
    {
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::Wildcard);
        if (!regExp.isValid())
        {
            QMessageBox::warning(this, tr("Error"), QString::fromUtf8("You have entered an invalid file name or expression."), QMessageBox::Ok);
            return;
        }
    }

    accept();
}

void AddExclusionDialog::on_bChoose_clicked()
{

#ifndef _WIN32
    QPointer<MultiQFileDialog> dialog = new MultiQFileDialog(0,  tr("Select local exclusion item"), QDir::home().path(), false);
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
    QString fPath = QFileDialog::getExistingDirectory(0,  tr("Select local exclusion item"), ui->eFolderPath->text());
#endif

    if (!fPath.size())
    {
        return;
    }

    ui->eExclusionItem->setText(fPath);
}

void AddExclusionDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
