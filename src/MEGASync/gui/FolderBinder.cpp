#include "FolderBinder.h"
#include "ui_FolderBinder.h"

#include "MegaApplication.h"
#include "QMegaMessageBox.h"
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

MegaHandle FolderBinder::selectedMegaFolder()
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
        delete selectedFolder;
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("You can not sync a shared folder without Full Access permissions"), QMessageBox::Ok);
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
        defaultPath = Utilities::getDefaultBasePath();
    }

    defaultPath = QDir::toNativeSeparators(defaultPath);

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Opening folder selector in: %1").arg(defaultPath).toUtf8().constData());
#ifndef _WIN32
    if (defaultPath.isEmpty())
    {
        defaultPath = QString::fromUtf8("/");
    }

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

        QTemporaryFile test(path + QDir::separator());
        if (test.open()|| QMegaMessageBox::warning(nullptr,
                                        tr("Warning"),
                                        tr("You don't have write permissions in this local folder.")
                                        + QString::fromUtf8("\n") + tr("MEGAsync won't be able to download anything here.")
                                        + QString::fromUtf8("\n") + tr("Do you want to continue?"),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        {
            ui->eLocalFolder->setText(path);
        }
    }
}

void FolderBinder::on_bMegaFolder_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(NodeSelector::SYNC_SELECT, this);
    MegaNode *defaultNode = megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData());
    if (defaultNode)
    {
        nodeSelector->setSelectedNodeHandle(defaultNode->getHandle());
        delete defaultNode;
    }

    int result = nodeSelector->exec();
    if (!nodeSelector || result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    MegaHandle selectedFolder = nodeSelector->getSelectedNodeHandle();
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
