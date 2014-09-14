#include "DownloadFromMegaDialog.h"
#include "ui_DownloadFromMegaDialog.h"
#include "control/Utilities.h"

#include <QDesktopServices>
#include <QFileDialog>

DownloadFromMegaDialog::DownloadFromMegaDialog(QWidget *parent) :
	QDialog(parent),
    ui(new Ui::DownloadFromMegaDialog)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_QuitOnClose, false);

    QString defaultDownloadPath;

#ifdef WIN32
    #if QT_VERSION < 0x050000
        defaultDownloadPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + tr("/MEGAsync Downloads");
    #else
        defaultDownloadPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0] + tr("/MEGAsync Downloads");
    #endif
#else
    #if QT_VERSION < 0x050000
        defaultDownloadPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + tr("/MEGAsync Downloads");
    #else
        defaultDownloadPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + tr("/MEGAsync Downloads");
    #endif
#endif

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

bool DownloadFromMegaDialog::isDefaultFolder()
{
    return ui->cDefaultPath->isChecked();
}

QString DownloadFromMegaDialog::getPath()
{
    return ui->eFolderPath->text();
}

void DownloadFromMegaDialog::on_bChange_clicked()
{
    QString fPath =  QFileDialog::getExistingDirectory(0, tr("Select local folder"),
                                                  ui->eFolderPath->text(),
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);

    if(fPath.size())
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
    QDir dir(ui->eFolderPath->text());
    if(!dir.exists())
        dir.mkpath(QString::fromUtf8("."));
    accept();
}
