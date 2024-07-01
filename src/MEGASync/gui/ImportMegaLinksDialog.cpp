#include "ImportMegaLinksDialog.h"
#include "ui_ImportMegaLinksDialog.h"
#include "ImportListWidgetItem.h"
#include "NodeSelectorSpecializations.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"
#include <MegaNodeNames.h>
#include "Platform.h"
#include "CommonMessages.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QPointer>

using namespace mega;

ImportMegaLinksDialog::ImportMegaLinksDialog(const QStringList& linkList, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ImportMegaLinksDialog)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mPreferences(Preferences::instance())
    , mDownloadPathChangedByUser(false)
    , mUseDefaultImportPath(true)
    , mImportPathChangedByUser(false)
{
    ui->setupUi(this);

    static const int MAX_ITEMS_DISPLAYED = 8;
    const int nbItems (linkList.size());
    mSelectedItems.resize(nbItems);

    for (int i = 0; i < nbItems; i++)
    {
        // Note: QWidget parents (in this case ui->linkList) will take ownership
        ImportListWidgetItem* customItem = new ImportListWidgetItem(linkList[i], i, ui->linkList);
        connect(customItem, &ImportListWidgetItem::stateChanged, this, &ImportMegaLinksDialog::onLinkStateChanged);
        QListWidgetItem *item = new QListWidgetItem(ui->linkList);
        ui->linkList->addItem(item);
        item->setSizeHint(customItem->size());
        ui->linkList->setItemWidget(item, customItem);
        mSelectedItems[i] = false;
    }

    int extraSlots = std::min(MAX_ITEMS_DISPLAYED, nbItems) - 1;
    ui->linkList->setFixedHeight(ui->linkList->minimumHeight() + ui->linkList->sizeHintForRow(0) * extraSlots);
    adjustSize();
    setFixedHeight(height());

    ui->linkList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    updateDownloadPath();

    if (mPreferences->logged())
    {
        initUiAsLogged();
    }
    else
    {
        initUiAsUnlogged();
    }

    mFinished = false;
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
    SelectorInfo info;

    info.defaultDir = ui->eLocalFolder->text().trimmed();
    if (!info.defaultDir.size())
    {
        info.defaultDir = Utilities::getDefaultBasePath();
    }

    info.defaultDir = QDir::toNativeSeparators(info.defaultDir);
    info.title = tr("Select local folder");
    info.multiSelection = false;
    info.parent = this;
    info.canCreateDirectories = true;
    info.func = [&](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            onLocalFolderSet(fPath);
        }
    };

    Platform::getInstance()->folderSelector(info);
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
            mDownloadPathChangedByUser = true;
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

            mImportPathChangedByUser = true;
            updateImportPath(QString::fromUtf8(fPath.get()));
        }
    });
}

void ImportMegaLinksDialog::setSelectedItem(int index, bool selected)
{
    if (index >= 0 && index < mSelectedItems.size())
    {
        mSelectedItems[index] = selected;
    }

}

void ImportMegaLinksDialog::onLinkInfoAvailable(int index,
                                                const QString& name,
                                                int status,
                                                long long size,
                                                bool isFolder)
{
    ImportListWidgetItem *item = (ImportListWidgetItem *)ui->linkList->itemWidget(ui->linkList->item(index));
    item->setData(name, static_cast<ImportListWidgetItem::linkstatus>(status), size, isFolder);
    item->updateGui();

    setSelectedItem(index, item->isSelected());
    emit linkSelected(index, item->isSelected());
}


void ImportMegaLinksDialog::onLinkInfoRequestFinish()
{
    mFinished = true;
    checkLinkValidAndSelected();
}

void ImportMegaLinksDialog::onLinkStateChanged(int index, int state)
{
    setSelectedItem(index, state);

    emit linkSelected(index, state);
    checkLinkValidAndSelected();
}

void ImportMegaLinksDialog::on_bOk_clicked()
{
    if (ui->cDownload->isChecked() && !ui->eLocalFolder->text().isEmpty())
    {
        QDir dir(ui->eLocalFolder->text());
        if (!dir.exists())
        {
            dir.mkpath(QLatin1String("."));
        }

        QString qFilePath = QDir::fromNativeSeparators(ui->eLocalFolder->text()); // QFile always wants `/` as separator
        QTemporaryFile test(qFilePath + QDir::separator());
        if (!test.open())
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("You don't have write permissions in this local folder.");
            QMegaMessageBox::critical(msgInfo);
            return;
        }
    }

    mPreferences->setImportMegaLinksEnabled(ui->cImport->isChecked());
    mPreferences->setDownloadMegaLinksEnabled(ui->cDownload->isChecked());

    accept();
}

void ImportMegaLinksDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        updateDownloadPath();
        updateImportPath();

        emit onChangeEvent();
        //! This mechanism was kept, but moved to LinkProcessor.cpp
        //! Not sure why this is needed?!
//!        int totalImports = mLinkProcessor->getCurrentIndex();
//!        for (int i = 0; i < totalImports; i++)
//!        {
//!            this->onLinkInfoAvailable(i);
//!        }
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
            mUseDefaultImportPath = false;
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
    mPreferences->setImportFolder(mega::INVALID_HANDLE);
    mUseDefaultImportPath = true;
    updateImportPath();
}

void ImportMegaLinksDialog::enableOkButton() const
{
    const bool downloadOrImportChecked{ui->cDownload->isChecked() || ui->cImport->isChecked()};
    const bool enable{mFinished && downloadOrImportChecked && (mSelectedItems.contains(true))};
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
    // Set enable to true if at least one item is selected
    bool enable = mSelectedItems.contains(true);
    ui->cDownload->setEnabled(enable);
    ui->cImport->setEnabled(enable);
    enableOkButton();
    enableLocalFolder(enable && ui->cDownload->isChecked());
    enableMegaFolder(enable && ui->cImport->isChecked());
}

void ImportMegaLinksDialog::updateDownloadPath()
{
    if (!mDownloadPathChangedByUser)
    {
        QString downloadFolder = mPreferences->downloadFolder();
        bool useDefaultDownloadPath = downloadFolder.isEmpty() || !QFileInfo::exists(downloadFolder);

        if (useDefaultDownloadPath)
        {
            downloadFolder = Utilities::getDefaultBasePath() + QLatin1Char('/')
                      + CommonMessages::getDefaultDownloadFolderName();
        }
        ui->eLocalFolder->setText(QDir::toNativeSeparators(downloadFolder));
    }
}

void ImportMegaLinksDialog::updateImportPath(const QString& path)
{
    QString newPath (path);
    if (!mImportPathChangedByUser && mUseDefaultImportPath)
    {
        newPath = QLatin1String("/") + CommonMessages::getDefaultImportFolderName();
    }

    if (!newPath.isEmpty())
    {
        ui->eMegaFolder->setText(newPath);
    }
}
