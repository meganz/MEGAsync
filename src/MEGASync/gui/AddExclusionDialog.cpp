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
        (!QFileInfo::exists(text) && !QRegExp(text, Qt::CaseInsensitive, QRegExp::Wildcard).isValid()))
    {
        QMegaMessageBox::MessageBoxInfo info;
        info.parent = this;
        info.title = QMegaMessageBox::warningTitle();
        info.text = tr("Please enter a valid file name or absolute path.");
        QMegaMessageBox::warning(info);
    }
    else
    {
        accept();
    }
}

void AddExclusionDialog::on_bChoose_clicked()
{
    auto processResult = [this](QStringList selection){
        if(!selection.isEmpty())
        {
            setTextToExclusionItem(selection.first());
        }
    };
    SelectorInfo info;
    info.defaultDir = QDir::home().path();
    info.multiSelection = false;
    info.parent = this;
    info.func = processResult;
#ifdef __APPLE__
    info.title = tr("Select the file or folder you want to exclude");
    Platform::getInstance()->fileAndFolderSelector(info);
#else
    info.title = tr("Select the folder you want to exclude");
    Platform::getInstance()->folderSelector(info);
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
                                          [this](QStringList selection){
        if(!selection.isEmpty())
        {
            ui->eExclusionItem->setText(QDir::toNativeSeparators(selection.first()));
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
