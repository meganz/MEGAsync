#include "DownloadFromMegaDialog.h"
#include "ui_DownloadFromMegaDialog.h"
#include "control/Utilities.h"
#include "gui/MultiQFileDialog.h"
#include "DialogOpener.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QTemporaryFile>
#include "QMegaMessageBox.h"
#include <QPointer>

DownloadFromMegaDialog::DownloadFromMegaDialog(QString defaultPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadFromMegaDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QString defaultDownloadPath;

    if (!defaultPath.size() || !QFile(defaultPath).exists())
    {
        defaultDownloadPath = Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGAsync Downloads");
    }
    else
    {
        defaultDownloadPath = defaultPath;
    }

    ui->eFolderPath->setText(QDir::toNativeSeparators(defaultDownloadPath));
    ui->cDefaultPath->setChecked(false);
    ui->bChange->setEnabled(true);
    ui->bOK->setEnabled(true);
    ui->bOK->setDefault(true);
}

DownloadFromMegaDialog::~DownloadFromMegaDialog()
{
    delete ui;
}

bool DownloadFromMegaDialog::isDefaultDownloadOption()
{
    return ui->cDefaultPath->isChecked();
}

void DownloadFromMegaDialog::setDefaultDownloadOption(bool value)
{
    ui->cDefaultPath->setChecked(value);
}

QString DownloadFromMegaDialog::getPath()
{
    return ui->eFolderPath->text();
}

void DownloadFromMegaDialog::on_bChange_clicked()
{
#ifndef _WIN32
    QPointer<MultiQFileDialog> fileDialog = new MultiQFileDialog(0,  tr("Select local folder"), ui->eFolderPath->text(), false);
    fileDialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    fileDialog->setFileMode(QFileDialog::DirectoryOnly);
    DialogOpener::showDialog<MultiQFileDialog>(fileDialog, [fileDialog, this](){
        if (fileDialog->result() == QDialog::Accepted && !fileDialog->selectedFiles().isEmpty())
        {
            QString fPath = fileDialog->selectedFiles().value(0);
            onPathChanged(fPath);
        }
    });
#else
    QString fPath = QFileDialog::getExistingDirectory(0,  tr("Select local folder"), ui->eFolderPath->text());
    onPathChanged(fPath);
#endif
}

void DownloadFromMegaDialog::onPathChanged(const QString& path)
{
    if (!path.size())
    {
        return;
    }

    QTemporaryFile test(path + QDir::separator());
    if (!test.open())
    {
        QMegaMessageBox::critical(nullptr, tr("Error"), tr("You don't have write permissions in this local folder."));
        return;
    }

    ui->eFolderPath->setText(path);
}

void DownloadFromMegaDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void DownloadFromMegaDialog::on_bOK_clicked()
{
    if (ui->eFolderPath->text().size())
    {
        QDir dir(ui->eFolderPath->text());
        if (!dir.exists())
        {
            dir.mkpath(QString::fromUtf8("."));
        }

        QString qFilePath = QDir::fromNativeSeparators(ui->eFolderPath->text()); // QFile always wants `/` as separator
        QTemporaryFile test(qFilePath + QDir::separator());
        if (!test.open())
        {
            QMegaMessageBox::critical(nullptr, tr("Error"), tr("You don't have write permissions in this local folder."));
            return;
        }

        accept();
    }
}
