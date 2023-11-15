#include "AddExclusionDialog.h"
#include "ui_AddExclusionDialog.h"
#include "QMegaMessageBox.h"
#include "Utilities.h"
#include "DialogOpener.h"
#include "Platform.h"

#include <QPointer>

AddExclusionDialog::AddExclusionDialog(const QString& syncLocalFolder, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddExclusionDialog),
    mSyncLocalFolder(syncLocalFolder)
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
    auto processResult = [this](const QStringList& selection){
        addItem(selection);
    };

#ifdef __APPLE__
    Platform::getInstance()->fileAndFolderSelector(tr("Select the file or folder you want to exclude"), mSyncLocalFolder, false, this, processResult);
#else
    Platform::getInstance()->folderSelector(tr("Select the folder you want to exclude"), mSyncLocalFolder, false, this, processResult);
#endif
}

#ifndef __APPLE__
void AddExclusionDialog::on_bChooseFile_clicked()
{
    auto processResult = [this](const QStringList& selection){
        addItem(selection);
    };

    Platform::getInstance()->fileSelector(tr("Select the file you want to exclude"), mSyncLocalFolder, false, this, processResult);
}
#endif

void AddExclusionDialog::addItem(const QStringList& selection)
{
    if (!selection.isEmpty())
    {
        const auto absolutePath = QDir::toNativeSeparators(selection.first());
        const auto relativePath = QDir (mSyncLocalFolder).relativeFilePath(absolutePath);
        ui->eExclusionItem->setText(relativePath);
    }
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

void AddExclusionDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
