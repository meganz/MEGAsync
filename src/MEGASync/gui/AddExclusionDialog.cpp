#include "AddExclusionDialog.h"
#include "ui_AddExclusionDialog.h"
#include "QMegaMessageBox.h"
#include "Utilities.h"
#include "DialogOpener.h"
#include "Platform.h"

#include <QPointer>

AddExclusionDialog::AddExclusionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddExclusionDialog)
{
    ui->setupUi(this);
    ui->bOk->setDefault(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
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
    if (text.isEmpty() ||
        (!QFileInfo::exists(text) && !QRegExp(text, Qt::CaseInsensitive, QRegExp::Wildcard).isValid())
       )
    {
        QMegaMessageBox::warning(this, tr("Warning"), tr("Please enter a valid file name or absolute path."));
        return;
    }

    accept();
}

void AddExclusionDialog::on_bChoose_clicked()
{
    auto processResult = [this](QStringList selection){
        if(!selection.isEmpty())
        {
            setTextToExclusionItem(selection.first());
        }
    };

#ifdef __APPLE__
    Platform::getInstance()->fileAndFolderSelector(tr("Select the file or folder you want to exclude"),QDir::home().path(), false, this, processResult);
#else
    Platform::getInstance()->fileAndFolderSelector(tr("Select the folder you want to exclude"),QDir::home().path(), false, this, processResult);
#endif
}

void AddExclusionDialog::setTextToExclusionItem(const QString& path)
{
    QPointer<AddExclusionDialog> currentDialog = this;
    if (!currentDialog || !path.size())
    {
        return;
    }

    ui->eExclusionItem->setText(QDir::toNativeSeparators(path));
}

#ifndef __APPLE__
void AddExclusionDialog::on_bChooseFile_clicked()
{
    Platform::getInstance()->fileSelector(tr("Select the file you want to exclude"),QDir::home().path(), false, this,
                                          [](QStringList selection){
        if(!selection.isEmpty())
        {
            ui->eExclusionItem->setText(QDir::toNativeSeparators(fPath));
        }
    });
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
