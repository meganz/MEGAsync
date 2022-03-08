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

ImportMegaLinksDialog::ImportMegaLinksDialog(MegaApi *megaApi, Preferences *preferences, LinkProcessor *processor, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportMegaLinksDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const int SLOT_HEIGHT = 35;
    this->megaApi = megaApi;
    this->preferences = preferences;
    this->linkProcessor = processor;

    for (int i = 0; i < linkProcessor->size(); i++)
    {
        ImportListWidgetItem *customItem = new ImportListWidgetItem(linkProcessor->getLink(i), i, ui->linkList);
        connect(customItem, SIGNAL(stateChanged(int,int)), this, SLOT(onLinkStateChanged(int, int)));
        QListWidgetItem *item = new QListWidgetItem(ui->linkList);
        ui->linkList->addItem(item);
        item->setSizeHint(customItem->size());
        ui->linkList->setItemWidget(item, customItem);
    }

    int extraSlots = linkProcessor->size() - 1;
    if (extraSlots > 7)
    {
        extraSlots = 7;
    }
    ui->linkList->setMinimumHeight(ui->linkList->minimumHeight() + SLOT_HEIGHT * extraSlots);
    this->setMinimumHeight(this->minimumHeight() + SLOT_HEIGHT * extraSlots);

    ui->linkList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setMaximumHeight(this->minimumHeight());

    QString defaultFolderPath;
    QString downloadFolder = QDir::toNativeSeparators(preferences->downloadFolder());
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

    if (preferences->logged())
    {
        initUiAsLogged(preferences);
    }
    else
    {
        initUiAsUnlogged();
    }

    connect(linkProcessor, SIGNAL(onLinkInfoAvailable(int)), this, SLOT(onLinkInfoAvailable(int)));
    connect(linkProcessor, SIGNAL(onLinkInfoRequestFinish()), this, SLOT(onLinkInfoRequestFinish()));

    finished = false;
    linkProcessor->requestLinkInfo();
    ui->bOk->setDefault(true);
    highDpiResize.init(this);
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
    if (path.length())
    {
        path = QDir::toNativeSeparators(path);
        QDir dir(path);
        if (!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
        {
            return;
        }

        QTemporaryFile test(path + QDir::separator());
        if (!test.open())
        {
            QMegaMessageBox::critical(nullptr, tr("Error"), tr("You don't have write permissions in this local folder."));
            return;
        }

        ui->eLocalFolder->setText(path);
    }
}

void ImportMegaLinksDialog::on_bMegaFolder_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(megaApi, NodeSelector::UPLOAD_SELECT, this);
    int result = nodeSelector->exec();
    if (!nodeSelector || result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *selectedFolder = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if (!selectedFolder)
    {
        delete nodeSelector;
        return;
    }

    const char *fPath = megaApi->getNodePath(selectedFolder);
    if (!fPath)
    {
        delete nodeSelector;
        delete selectedFolder;
        return;
    }

    ui->eMegaFolder->setText(QString::fromUtf8(fPath));
    delete nodeSelector;
    delete selectedFolder;
    delete [] fPath;
}

void ImportMegaLinksDialog::onLinkInfoAvailable(int id)
{
    ImportListWidgetItem *item = (ImportListWidgetItem *)ui->linkList->itemWidget(ui->linkList->item(id));
    std::shared_ptr<MegaNode> node = linkProcessor->getNode(id);

    int e = linkProcessor->getError(id);
    if (node && (e == MegaError::API_OK))
    {
        QString name = QString::fromUtf8(node->getName());
        if (!name.compare(QString::fromAscii("NO_KEY")) || !name.compare(QString::fromAscii("CRYPTO_ERROR")))
        {
            item->setData(tr("Decryption error"), ImportListWidgetItem::WARNING, megaApi->getSize(node.get()), !(node->getType() == MegaNode::TYPE_FILE));
        }
        else
        {
            item->setData(name, ImportListWidgetItem::CORRECT, megaApi->getSize(node.get()), !(node->getType() == MegaNode::TYPE_FILE));
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
    linkProcessor->setSelected(id, item->isSelected());
}

void ImportMegaLinksDialog::onLinkInfoRequestFinish()
{
    finished = true;
    checkLinkValidAndSelected();
}

void ImportMegaLinksDialog::onLinkStateChanged(int id, int state)
{
    linkProcessor->setSelected(id, state);
    checkLinkValidAndSelected();
}

void ImportMegaLinksDialog::accept()
{
    preferences->setImportMegaLinksEnabled(ui->cImport->isChecked());
    preferences->setDownloadMegaLinksEnabled(ui->cDownload->isChecked());
    QDialog::accept();
}

void ImportMegaLinksDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        int totalImports = linkProcessor->getCurrentIndex();
        for (int i = 0; i < totalImports; i++)
            this->onLinkInfoAvailable(i);
    }
    QDialog::changeEvent(event);
}

void ImportMegaLinksDialog::initUiAsLogged(Preferences* preferences)
{
    initImportFolderControl(preferences);
    ui->cImport->setChecked(preferences->getImportMegaLinksEnabled());
    ui->cDownload->setChecked(preferences->getDownloadMegaLinksEnabled());
}

void ImportMegaLinksDialog::initUiAsUnlogged()
{
    ui->cImport->setChecked(false);
    ui->cImport->setVisible(false);
    ui->wImport->setVisible(false);
    ui->cDownload->setVisible(false);
}

void ImportMegaLinksDialog::initImportFolderControl(Preferences* preferences)
{
    std::unique_ptr<MegaNode> importFolderNode(megaApi->getNodeByHandle(preferences->importFolder()));
    if (importFolderNode)
    {
        const char *importFolderPath = megaApi->getNodePath(importFolderNode.get());
        if (importFolderPath && strncmp(importFolderPath, "//bin/", 6))
        {
            ui->eMegaFolder->setText(QString::fromUtf8(importFolderPath));
            delete [] importFolderPath;
        }
        else
        {
            setInvalidImportFolder(preferences);
        }
    }
    else
    {
        setInvalidImportFolder(preferences);
    }
}

void ImportMegaLinksDialog::setInvalidImportFolder(Preferences *preferences)
{
    ui->eMegaFolder->setText(QString::fromUtf8("/MEGAsync Imports"));
    preferences->setImportFolder(mega::INVALID_HANDLE);
}

void ImportMegaLinksDialog::enableOkButton() const
{
    const bool downloadOrImportChecked{ui->cDownload->isChecked() || ui->cImport->isChecked()};
    const bool enable{finished && downloadOrImportChecked && linkProcessor->atLeastOneLinkValidAndSelected()};
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
    const bool enable{linkProcessor->atLeastOneLinkValidAndSelected()};
    ui->cDownload->setEnabled(enable);
    ui->cImport->setEnabled(enable);
    enableOkButton();
    enableLocalFolder(enable && ui->cDownload->isChecked());
    enableMegaFolder(enable && ui->cImport->isChecked());
}
