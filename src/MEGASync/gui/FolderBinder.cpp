#include "FolderBinder.h"
#include "ui_FolderBinder.h"

#include "MegaApplication.h"
#include "control/Utilities.h"

using namespace mega;

FolderBinder::FolderBinder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderBinder)
{
    selectedMegaFolderHandle = mega::INVALID_HANDLE;
    ui->setupUi(this);
    app = (MegaApplication *)qApp;
    megaApi = app->getMegaApi();
}

FolderBinder::~FolderBinder()
{
    delete ui;
}

long long FolderBinder::selectedMegaFolder()
{
    return selectedMegaFolderHandle;
}

bool FolderBinder::setSelectedMegaFolder(MegaHandle handle)
{
    MegaNode *selectedFolder = megaApi->getNodeByHandle(handle);
    if (!selectedFolder)
    {
        selectedMegaFolderHandle = mega::INVALID_HANDLE;
        return false;
    }

    if (megaApi->getAccess(selectedFolder) < MegaShare::ACCESS_FULL)
    {
        selectedMegaFolderHandle = mega::INVALID_HANDLE;
        QMessageBox::warning(this, tr("Error"), tr("You can not sync a shared folder without Full Access permissions"), QMessageBox::Ok);
        delete selectedFolder;
        return false;
    }

    const char *fPath = megaApi->getNodePath(selectedFolder);
    if (!fPath)
    {
        selectedMegaFolderHandle = mega::INVALID_HANDLE;
        delete selectedFolder;
        return false;
    }

    selectedMegaFolderHandle = handle;
    ui->eMegaFolder->setText(QString::fromUtf8(fPath));

    delete selectedFolder;
    delete [] fPath;
    return true;
}

QString FolderBinder::selectedLocalFolder()
{
    return ui->eLocalFolder->text();
}

void FolderBinder::on_bLocalFolder_clicked()
{
    QString defaultPath = ui->eLocalFolder->text().trimmed();
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Default path: %1").arg(defaultPath).toUtf8().constData());
    if (!defaultPath.size())
    {
#ifdef WIN32
    #if QT_VERSION < 0x050000
        defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    #else
        defaultPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
    #endif
#else
    #if QT_VERSION < 0x050000
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Getting home path (QT4)");
        defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Result: %1").arg(defaultPath).toUtf8().constData());
    #else
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Getting home path (QT5)");
        defaultPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Result: %1").arg(defaultPath).toUtf8().constData());
    #endif
#endif
    }

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Opening folder selector in: %1").arg(defaultPath).toUtf8().constData());
#ifndef _WIN32
    QPointer<MultiQFileDialog> dialog = new MultiQFileDialog(0,  tr("Select local folder"), defaultPath, false);
    dialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    dialog->setFileMode(QFileDialog::DirectoryOnly);
    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted || dialog->selectedFiles().isEmpty())
    {
        delete dialog;
        return;
    }
    QString path = dialog->selectedFiles().value(0);
    delete dialog;
#else
    QString path = QFileDialog::getExistingDirectory(0,  tr("Select local folder"), defaultPath);
#endif

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Folder selector closed. Result: %1").arg(path).toUtf8().constData());
    if (path.length())
    {        
        QDir dir(path);
        if (!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
        {
            return;
        }

        path = QDir::toNativeSeparators(path);
        if (!Utilities::verifySyncedFolderLimits(path))
        {
            QMessageBox::warning(this, tr("Warning"), tr("You are trying to sync an extremely large folder.\nTo prevent the syncing of entire boot volumes, which is inefficient and dangerous,\nwe ask you to start with a smaller folder and add more data while MEGAsync is running."), QMessageBox::Ok);
            return;
        }

        QTemporaryFile test(path + QDir::separator());
        if (test.open()
                || QMessageBox::warning(window(),
                                        tr("Warning"),
                                        tr("You don't have write permissions in this local folder.")
                                        + QString::fromUtf8("\n") + tr("MEGAsync won't be able to download anything here.")
                                        + QString::fromUtf8("\n") + tr("Do you want to continue?"),
                                        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            ui->eLocalFolder->setText(path);
        }
    }
}

void FolderBinder::on_bMegaFolder_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(megaApi, NodeSelector::SYNC_SELECT, this);
    int result = nodeSelector->exec();
    if (!nodeSelector || result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    MegaHandle selectedFolder = nodeSelector->getSelectedFolderHandle();
    setSelectedMegaFolder(selectedFolder);
    delete nodeSelector;
}

void FolderBinder::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
