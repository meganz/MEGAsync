#include "ImportMegaLinksDialog.h"
#include "ui_ImportMegaLinksDialog.h"
#include "gui/ImportListWidgetItem.h"
#include "gui/node_selector/gui/NodeSelector.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"
#include <MegaNodeNames.h>
#include "Platform.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QPointer>

using namespace mega;

ImportMegaLinksDialog::ImportMegaLinksDialog(LinkProcessor *linkProcessor, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportMegaLinksDialog),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mPreferences(Preferences::instance()),
    mLinkProcessor(linkProcessor)
{
    ui->setupUi(this);

    static const int MAX_ITEMS_DISPLAYED = 8;
    const int nbItems (mLinkProcessor->size());

    for (int i = 0; i < nbItems; i++)
    {
        ImportListWidgetItem *customItem = new ImportListWidgetItem(mLinkProcessor->getLink(i), i, ui->linkList);
        connect(customItem, SIGNAL(stateChanged(int,int)), this, SLOT(onLinkStateChanged(int, int)));
        QListWidgetItem *item = new QListWidgetItem(ui->linkList);
        ui->linkList->addItem(item);
        item->setSizeHint(customItem->size());
        ui->linkList->setItemWidget(item, customItem);
    }

    int extraSlots = std::min(MAX_ITEMS_DISPLAYED, nbItems) - 1;
    ui->linkList->setFixedHeight(ui->linkList->minimumHeight() + ui->linkList->sizeHintForRow(0) * extraSlots);
    adjustSize();
    setFixedHeight(height());

    ui->linkList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QString defaultFolderPath;
    QString downloadFolder = QDir::toNativeSeparators(mPreferences->downloadFolder());
    QFileInfo test(downloadFolder);
    if (!test.isDir())
    {
        QDir defaultFolder(QDir::toNativeSeparators(Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGA Downloads")));
        defaultFolder.mkpath(QString::fromLatin1("."));
        defaultFolderPath = defaultFolder.absolutePath();
        defaultFolderPath = QDir::toNativeSeparators(defaultFolderPath);
    }
    else
    {
        defaultFolderPath = downloadFolder;
    }

    ui->eLocalFolder->setText(defaultFolderPath);

    if (mPreferences->logged())
    {
        initUiAsLogged();
    }
    else
    {
        initUiAsUnlogged();
    }

    mLinkProcessor->setParentHandler(this);
    connect(mLinkProcessor, &LinkProcessor::onLinkInfoAvailable, this, &ImportMegaLinksDialog::onLinkInfoAvailable);
    connect(mLinkProcessor, &LinkProcessor::onLinkInfoRequestFinish, this, &ImportMegaLinksDialog::onLinkInfoRequestFinish);

    mFinished = false;
    mLinkProcessor->requestLinkInfo();
    ui->bOk->setDefault(true);
}

ImportMegaLinksDialog::~ImportMegaLinksDialog()
{
    delete ui;
}

bool ImportMegaLinksDialog::shouldImport()
{
    return ui->cImport->isChecked();
}

bool ImportMegaLinksDialog::shouldDownload()
{
    return ui->cDownload->isChecked();
}

QString ImportMegaLinksDialog::getImportPath()
{
    return ui->eMegaFolder->text();
}

QString ImportMegaLinksDialog::getDownloadPath()
{
    return QDir::toNativeSeparators(ui->eLocalFolder->text());
}

void ImportMegaLinksDialog::on_cDownload_clicked()
{
    enableLocalFolder(ui->cDownload->isChecked());
    enableOkButton();
}

void ImportMegaLinksDialog::on_cImport_clicked()
{
    enableMegaFolder(ui->cImport->isChecked());
    enableOkButton();
}

void ImportMegaLinksDialog::on_bLocalFolder_clicked()
{
    QString defaultPath = ui->eLocalFolder->text().trimmed();
    if (!defaultPath.size())
    {
        defaultPath = Utilities::getDefaultBasePath();
    }

    defaultPath = QDir::toNativeSeparators(defaultPath);

    Platform::getInstance()->folderSelector(tr("Select local folder"),defaultPath,false,this,[this](const QStringList& selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            onLocalFolderSet(fPath);
        }
    });
}

void ImportMegaLinksDialog::onLocalFolderSet(const QString& path)
{
    if (path.length())
    {
        auto nativePath = QDir::toNativeSeparators(path);
        QDir dir(nativePath);
        if (!dir.exists() && !dir.mkpath(QString::fromLatin1(".")))
        {
            return;
        }

        QTemporaryFile test(nativePath + QDir::separator());
        if (!test.open())
        {
            QMegaMessageBox::MessageBoxInfo info;
            info.title = QMegaMessageBox::errorTitle();
            info.text = tr("You don't have write permissions in this local folder.");
            info.parent = this;
            QMegaMessageBox::critical(info);
        }
        else
        {
            ui->eLocalFolder->setText(nativePath);
        }
    }
}

void ImportMegaLinksDialog::on_bMegaFolder_clicked()
{
    UploadNodeSelector* nodeSelector = new UploadNodeSelector(this);
    DialogOpener::showDialog<NodeSelector>(nodeSelector,[this, nodeSelector](){
        if (nodeSelector->result() == QDialog::Accepted)
        {
            MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedNodeHandle();
            std::shared_ptr<MegaNode> selectedFolder(mMegaApi->getNodeByHandle(selectedMegaFolderHandle));
            if (!selectedFolder)
            {
                return;
            }

            std::unique_ptr<const char[]> fPath(mMegaApi->getNodePath(selectedFolder.get()));
            if (!fPath)
            {
                return;
            }

            ui->eMegaFolder->setText(QString::fromUtf8(fPath.get()));
        }
    });
}

void ImportMegaLinksDialog::onLinkInfoAvailable(int id)
{
    ImportListWidgetItem *item = (ImportListWidgetItem *)ui->linkList->itemWidget(ui->linkList->item(id));
    std::shared_ptr<MegaNode> node = mLinkProcessor->getNode(id);

    int e = mLinkProcessor->getError(id);
    if (node && (e == MegaError::API_OK))
    {
        if (!node->isNodeKeyDecrypted())
        {
            item->setData(MegaNodeNames::getNodeName(node.get()), ImportListWidgetItem::WARNING, mMegaApi->getSize(node.get()), !(node->getType() == MegaNode::TYPE_FILE));
        }
        else
        {
            item->setData(QString::fromUtf8(node->getName()), ImportListWidgetItem::CORRECT, mMegaApi->getSize(node.get()), !(node->getType() == MegaNode::TYPE_FILE));
        }
    }
    else
    {
        if ((e != MegaError::API_OK) && (e != MegaError::API_ETOOMANY))
        {
            ImportListWidgetItem::linkstatus status = ImportListWidgetItem::FAILED;
            if (e == MegaError::API_ETEMPUNAVAIL)
            {
                status = ImportListWidgetItem::WARNING;
            }
            item->setData(QCoreApplication::translate("MegaError", MegaError::getErrorString(e, MegaError::API_EC_IMPORT)), status);
        }
        else
        {
            item->setData(tr("Not found"), ImportListWidgetItem::FAILED);
        }
    }
    item->updateGui();
    mLinkProcessor->setSelected(id, item->isSelected());
}

void ImportMegaLinksDialog::onLinkInfoRequestFinish()
{
    mFinished = true;
    checkLinkValidAndSelected();
}

void ImportMegaLinksDialog::onLinkStateChanged(int id, int state)
{
    mLinkProcessor->setSelected(id, state);
    checkLinkValidAndSelected();
}

void ImportMegaLinksDialog::accept()
{
    mPreferences->setImportMegaLinksEnabled(ui->cImport->isChecked());
    mPreferences->setDownloadMegaLinksEnabled(ui->cDownload->isChecked());
    QDialog::accept();
}

void ImportMegaLinksDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        int totalImports = mLinkProcessor->getCurrentIndex();
        for (int i = 0; i < totalImports; i++)
            this->onLinkInfoAvailable(i);
    }
    QDialog::changeEvent(event);
}

void ImportMegaLinksDialog::initUiAsLogged()
{
    initImportFolderControl();
    ui->cImport->setChecked(mPreferences->getImportMegaLinksEnabled());
    ui->cDownload->setChecked(mPreferences->getDownloadMegaLinksEnabled());
}

void ImportMegaLinksDialog::initUiAsUnlogged()
{
    ui->cImport->setChecked(false);
    ui->cImport->setVisible(false);
    ui->wImport->setVisible(false);
    ui->cDownload->setVisible(false);
}

void ImportMegaLinksDialog::initImportFolderControl()
{
    std::unique_ptr<MegaNode> importFolderNode(mMegaApi->getNodeByHandle(mPreferences->importFolder()));
    if (importFolderNode)
    {
        const char *importFolderPath = mMegaApi->getNodePath(importFolderNode.get());
        if (importFolderPath && strncmp(importFolderPath, "//bin/", 6))
        {
            ui->eMegaFolder->setText(QString::fromUtf8(importFolderPath));
            delete [] importFolderPath;
        }
        else
        {
            setInvalidImportFolder();
        }
    }
    else
    {
        setInvalidImportFolder();
    }
}

void ImportMegaLinksDialog::setInvalidImportFolder()
{
    ui->eMegaFolder->setText(QString::fromUtf8("/MEGA Imports"));
    mPreferences->setImportFolder(mega::INVALID_HANDLE);
}

void ImportMegaLinksDialog::enableOkButton() const
{
    const bool downloadOrImportChecked{ui->cDownload->isChecked() || ui->cImport->isChecked()};
    const bool enable{mFinished && downloadOrImportChecked && mLinkProcessor->atLeastOneLinkValidAndSelected()};
    ui->bOk->setEnabled(enable);
}

void ImportMegaLinksDialog::enableLocalFolder(bool enable)
{
    ui->bLocalFolder->setEnabled(enable);
    ui->eLocalFolder->setEnabled(enable);
}

void ImportMegaLinksDialog::enableMegaFolder(bool enable)
{
    ui->bMegaFolder->setEnabled(enable);
    ui->eMegaFolder->setEnabled(enable);
}

void ImportMegaLinksDialog::checkLinkValidAndSelected()
{
    const bool enable{mLinkProcessor->atLeastOneLinkValidAndSelected()};
    ui->cDownload->setEnabled(enable);
    ui->cImport->setEnabled(enable);
    enableOkButton();
    enableLocalFolder(enable && ui->cDownload->isChecked());
    enableMegaFolder(enable && ui->cImport->isChecked());
}
