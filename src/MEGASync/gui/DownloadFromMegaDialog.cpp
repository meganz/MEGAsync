#include "DownloadFromMegaDialog.h"
#include "ui_DownloadFromMegaDialog.h"
#include "control/Utilities.h"
#include "Platform.h"
#include "CommonMessages.h"

#include <QDesktopServices>
#include <QTemporaryFile>
#include "QMegaMessageBox.h"
#include <QPointer>

DownloadFromMegaDialog::DownloadFromMegaDialog(QString path, QWidget *parent) :
    QDialog(parent),
    mUseDefaultPath(false),
    mPathChangedByUser(false),
    ui(new Ui::DownloadFromMegaDialog)
{
    ui->setupUi(this);

    mUseDefaultPath = path.isEmpty() || !QFile(path).exists();

    if (mUseDefaultPath)
    {
        updatePath();
    }
    else
    {
        ui->eFolderPath->setText(QDir::toNativeSeparators(path));
    }

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
    SelectorInfo info;
    info.title = tr("Select local folder");
    info.defaultDir = ui->eFolderPath->text();
    info.multiSelection = false;
    info.canCreateDirectories = true;
    info.parent = this;
    info.func = [&](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            onPathChanged(fPath);
        }
    };

    Platform::getInstance()->folderSelector(info);
}

void DownloadFromMegaDialog::onPathChanged(const QString& path)
{
    if (path.isEmpty())
    {
        return;
    }

    QTemporaryFile test(path + QDir::separator());
    if (!test.open())
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = QMegaMessageBox::errorTitle();
        msgInfo.text = tr("You don't have write permissions in this local folder.");

        QMegaMessageBox::critical(msgInfo);
    }
    else
    {
        ui->eFolderPath->setText(QDir::toNativeSeparators(path));
        mPathChangedByUser = true;
    }
}

void DownloadFromMegaDialog::updatePath()
{
    if (!mPathChangedByUser && mUseDefaultPath)
    {
        auto downloadPath = Utilities::getDefaultBasePath() + QLatin1Char('/')
                            + CommonMessages::getDefaultDownloadFolderName();
        ui->eFolderPath->setText(QDir::toNativeSeparators(downloadPath));
    }
}

void DownloadFromMegaDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        updatePath();
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
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("You don't have write permissions in this local folder.");
            QMegaMessageBox::critical(msgInfo);
        }
        else
        {
            accept();
        }
    }
}
