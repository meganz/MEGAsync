#include "FolderBinder.h"
#include "ui_FolderBinder.h"
#include "node_selector/gui/NodeSelectorSpecializations.h"

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

    setFocusProxy(ui->bLocalFolder);
}

FolderBinder::~FolderBinder()
{
    delete ui;
}

MegaHandle FolderBinder::selectedMegaFolder()
{
    return selectedMegaFolderHandle;
}

void FolderBinder::setSelectedMegaFolder(MegaHandle handle)
{
    std::unique_ptr<MegaNode> selectedFolder(megaApi->getNodeByHandle(handle));
    if (!selectedFolder)
    {
        selectedMegaFolderHandle = mega::INVALID_HANDLE;
    }
    else
    {
        if (megaApi->getAccess(selectedFolder.get()) < MegaShare::ACCESS_FULL)
        {
            selectedMegaFolderHandle = mega::INVALID_HANDLE;

            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("You can not sync a shared folder without Full Access permissions");
            QMegaMessageBox::warning(msgInfo);
        }
        else
        {
            std::unique_ptr<char[]>fPath(megaApi->getNodePath(selectedFolder.get()));
            if (!fPath)
            {
                selectedMegaFolderHandle = mega::INVALID_HANDLE;
            }
            else
            {
                selectedMegaFolderHandle = handle;
                ui->eMegaFolder->setText(QString::fromUtf8(fPath.get()));

                checkSelectedSides();
            }
        }
    }
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

    SelectorInfo info;
    info.title = tr("Select local folder");
    info.defaultDir = defaultPath;
    info.multiSelection = false;
    info.parent = this;
    info.func = [this](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            onLocalFolderSet(fPath);
        }
    };

    Platform::getInstance()->folderSelector(info);
}

void FolderBinder::onLocalFolderSet(const QString& path)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Folder selector closed. Result: %1").arg(path).toUtf8().constData());
    if (QDir(path).exists())
    {
        auto nativePath = QDir::toNativeSeparators(path);
        ui->eLocalFolder->setText(nativePath);

        checkSelectedSides();
    }
}

void FolderBinder::checkSelectedSides()
{
    if(ui->eLocalFolder->text().isEmpty())
    {
        ui->bLocalFolder->setFocus();
    }
    else if(ui->eMegaFolder->text().isEmpty())
    {
        ui->bMegaFolder->setFocus();
    }
    else
    {
        emit selectionDone();
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

void FolderBinder::showEvent(QShowEvent *event)
{
    checkSelectedSides();

    QWidget::showEvent(event);
}
