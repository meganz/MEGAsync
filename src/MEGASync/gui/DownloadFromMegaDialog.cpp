#include "DownloadFromMegaDialog.h"
#include "ui_DownloadFromMegaDialog.h"
#include "control/Utilities.h"
#include "gui/MultiQFileDialog.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QPointer>

DownloadFromMegaDialog::DownloadFromMegaDialog(QString defaultPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadFromMegaDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    QString defaultDownloadPath;

    if (!defaultPath.size() || !QFile(defaultPath).exists())
    {
#ifdef WIN32
    #if QT_VERSION < 0x050000
        defaultDownloadPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QString::fromUtf8("/MEGAsync Downloads");
    #else
        defaultDownloadPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0] + QString::fromUtf8("/MEGAsync Downloads");
    #endif
#else
    #if QT_VERSION < 0x050000
        defaultDownloadPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + QString::fromUtf8("/MEGAsync Downloads");
    #else
        defaultDownloadPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + QString::fromUtf8("/MEGAsync Downloads");
    #endif
#endif
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
    QPointer<MultiQFileDialog> dialog = new MultiQFileDialog(0,  tr("Select local folder"), ui->eFolderPath->text(), false);
    dialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    dialog->setFileMode(QFileDialog::DirectoryOnly);
    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted || dialog->selectedFiles().isEmpty())
    {
        delete dialog;
        return;
    }
    QString fPath = dialog->selectedFiles().value(0);
    delete dialog;
#else
    QString fPath = QFileDialog::getExistingDirectory(0,  tr("Select local folder"), ui->eFolderPath->text());
#endif

    if (!fPath.size())
    {
        return;
    }

    QTemporaryFile test(fPath + QDir::separator());
    if (!test.open())
    {
        QMessageBox::critical(window(), tr("Error"), tr("You don't have write permissions in this local folder."));
        return;
    }

    ui->eFolderPath->setText(fPath);
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

        QTemporaryFile test(ui->eFolderPath->text() + QDir::separator());
        if (!test.open())
        {
            QMessageBox::critical(window(), tr("Error"), tr("You don't have write permissions in this local folder."));
            return;
        }

        accept();
    }
}
