#include "FolderBinder.h"
#include "ui_FolderBinder.h"

#include "MegaApplication.h"
#include "control/Utilities.h"
#include "DialogOpener.h"
#include "Platform.h"

#include "QMegaMessageBox.h"

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

    Platform::getInstance()->folderSelector(tr("Select local folder"),defaultPath,false,this,[this](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            onLocalFolderSet(fPath);
        }
    });
}

void FolderBinder::onLocalFolderSet(const QString& path)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Folder selector closed. Result: %1").arg(path).toUtf8().constData());
    if (QDir(path).exists())
    {
        auto nativePath = QDir::toNativeSeparators(path);
        ui->eLocalFolder->setText(nativePath);
    }
}

void FolderBinder::on_bMegaFolder_clicked()
{
    QPointer<SyncNodeSelector> nodeSelector = new SyncNodeSelector(this);
    std::shared_ptr<MegaNode> defaultNode(megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData()));
    nodeSelector->setSelectedNodeHandle(defaultNode);

    DialogOpener::showDialog<SyncNodeSelector>(nodeSelector, [nodeSelector, this]()
    {
        if (nodeSelector->result() == QDialog::Accepted)
        {
            MegaHandle selectedFolder = nodeSelector->getSelectedNodeHandle();
            setSelectedMegaFolder(selectedFolder);
        }
    });
}

void FolderBinder::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
