#include "ImportMegaLinksDialog.h"
#include "ui_ImportMegaLinksDialog.h"
#include "gui/ImportListWidgetItem.h"
#include "gui/NodeSelector.h"
#include "gui/MultiQFileDialog.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QPointer>

using namespace mega;

ImportMegaLinksDialog::ImportMegaLinksDialog(std::shared_ptr<LinkProcessor> linkProcessor, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportMegaLinksDialog),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mPreferences(Preferences::instance()),
    mLinkProcessor(linkProcessor)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
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
        QDir defaultFolder(QDir::toNativeSeparators(Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGAsync Downloads")));
        defaultFolder.mkpath(QString::fromAscii("."));
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
        initUiAsLogged(mPreferences);
    }
    else
    {
        initUiAsUnlogged();
    }

    connect(mLinkProcessor.get(), &LinkProcessor::onLinkInfoAvailable, this, &ImportMegaLinksDialog::onLinkInfoAvailable);
    connect(mLinkProcessor.get(), &LinkProcessor::onLinkInfoRequestFinish, this, &ImportMegaLinksDialog::onLinkInfoRequestFinish);

    finished = false;
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

#ifndef _WIN32
    if (defaultPath.isEmpty())
    {
        defaultPath = QString::fromUtf8("/");
    }

    QPointer<MultiQFileDialog> fileDialog = new MultiQFileDialog(0,  tr("Select local folder"), defaultPath, false);
    fileDialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    fileDialog->setFileMode(QFileDialog::DirectoryOnly);
    Utilities::showDialog<MultiQFileDialog>(fileDialog, [fileDialog, this]()
    {
        if (fileDialog->result() == QDialog::Accepted && !fileDialog->selectedFiles().isEmpty())
        {
            QString path = fileDialog->selectedFiles().value(0);
            onLocalFolderSet(path);
        }
    });
#else
    QString path = QFileDialog::getExistingDirectory(0,  tr("Select local folder"), defaultPath);
    onLocalFolderSet(path);
#endif
}

void ImportMegaLinksDialog::onLocalFolderSet(const QString& path)
{
    if (path.length())
    {
        auto nativePath = QDir::toNativeSeparators(path);
        QDir dir(nativePath);
        if (!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
        {
            return;
        }

        QTemporaryFile test(nativePath + QDir::separator());
        if (!test.open())
        {
            QMegaMessageBox::critical(nullptr, tr("Error"), tr("You don't have write permissions in this local folder."));
            return;
        }

        ui->eLocalFolder->setText(nativePath);
    }
}

void ImportMegaLinksDialog::on_bMegaFolder_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(NodeSelector::UPLOAD_SELECT, this);
    Utilities::showDialog(nodeSelector, this, &ImportMegaLinksDialog::onMegaFolderSelectorFinished);
}

void ImportMegaLinksDialog::onMegaFolderSelectorFinished(QPointer<NodeSelector> nodeSelector)
{
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
}

void ImportMegaLinksDialog::onLinkInfoAvailable(int id)
{
    ImportListWidgetItem *item = (ImportListWidgetItem *)ui->linkList->itemWidget(ui->linkList->item(id));
    std::shared_ptr<MegaNode> node = mLinkProcessor->getNode(id);

    int e = mLinkProcessor->getError(id);
    if (node && (e == MegaError::API_OK))
    {
        QString name = QString::fromUtf8(node->getName());
        if (!name.compare(QLatin1String("NO_KEY")) || !name.compare(QLatin1String("CRYPTO_ERROR")))
        {
            item->setData(QCoreApplication::translate("MegaError", "Decryption error"), ImportListWidgetItem::WARNING, mMegaApi->getSize(node.get()), !(node->getType() == MegaNode::TYPE_FILE));
        }
        else
        {
            item->setData(name, ImportListWidgetItem::CORRECT, mMegaApi->getSize(node.get()), !(node->getType() == MegaNode::TYPE_FILE));
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
    finished = true;
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

void ImportMegaLinksDialog::initUiAsLogged(std::shared_ptr<Preferences> mPreferences)
{
    initImportFolderControl(mPreferences);
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

void ImportMegaLinksDialog::initImportFolderControl(std::shared_ptr<Preferences> mPreferences)
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
            setInvalidImportFolder(mPreferences);
        }
    }
    else
    {
        setInvalidImportFolder(mPreferences);
    }
}

void ImportMegaLinksDialog::setInvalidImportFolder(std::shared_ptr<Preferences> mPreferences)
{
    ui->eMegaFolder->setText(QString::fromUtf8("/MEGAsync Imports"));
    mPreferences->setImportFolder(mega::INVALID_HANDLE);
}

void ImportMegaLinksDialog::enableOkButton() const
{
    const bool downloadOrImportChecked{ui->cDownload->isChecked() || ui->cImport->isChecked()};
    const bool enable{finished && downloadOrImportChecked && mLinkProcessor->atLeastOneLinkValidAndSelected()};
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
