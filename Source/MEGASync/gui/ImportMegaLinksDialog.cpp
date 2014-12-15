#include "ImportMegaLinksDialog.h"
#include "ui_ImportMegaLinksDialog.h"
#include "gui/ImportListWidgetItem.h"
#include "gui/NodeSelector.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTemporaryFile>

using namespace mega;

ImportMegaLinksDialog::ImportMegaLinksDialog(MegaApi *megaApi, Preferences *preferences, LinkProcessor *processor, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ImportMegaLinksDialog)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_QuitOnClose, false);

	this->megaApi = megaApi;
	this->linkProcessor = processor;

	for(int i=0; i<linkProcessor->size(); i++)
	{
		ImportListWidgetItem *customItem = new ImportListWidgetItem(linkProcessor->getLink(i), i, ui->linkList);
		connect(customItem, SIGNAL(stateChanged(int,int)), this, SLOT(onLinkStateChanged(int, int)));
		QListWidgetItem *item = new QListWidgetItem(ui->linkList);
		ui->linkList->addItem(item);
		item->setSizeHint(customItem->size());
		ui->linkList->setItemWidget(item, customItem);
	}

	if(linkProcessor->size()<=8)
	{
        ui->linkList->setMinimumHeight(ui->linkList->minimumHeight()+35*(linkProcessor->size()-1));
        this->setMinimumHeight(this->minimumHeight()+35*(linkProcessor->size()-1));
	}
	else
	{
        ui->linkList->setMinimumHeight(ui->linkList->minimumHeight()+35*7);
        this->setMinimumHeight(this->minimumHeight()+35*7);
	}

	ui->linkList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setMaximumHeight(this->minimumHeight());

	QString defaultFolderPath;
	QFileInfo test(preferences->downloadFolder());
	if(!test.isDir())
	{
        #ifdef WIN32
            #if QT_VERSION < 0x050000
                QDir defaultFolder(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QString::fromUtf8("/MEGAsync Downloads"));
            #else
                QDir defaultFolder(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0] + QString::fromUtf8("/MEGAsync Downloads"));
            #endif
        #else
            #if QT_VERSION < 0x050000
                QDir defaultFolder(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + QString::fromUtf8("/MEGAsync Downloads"));
            #else
                QDir defaultFolder(QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + QString::fromUtf8("/MEGAsync Downloads"));
            #endif
        #endif

        defaultFolder.mkpath(QString::fromAscii("."));
		defaultFolderPath = defaultFolder.absolutePath();
		defaultFolderPath = QDir::toNativeSeparators(defaultFolderPath);
	}
	else defaultFolderPath = preferences->downloadFolder();
	ui->eLocalFolder->setText(defaultFolderPath);

    MegaNode *testNode = megaApi->getNodeByHandle(preferences->importFolder());
    if(testNode)
    {
        const char *tPath = megaApi->getNodePath(testNode);
        if(tPath)
        {
            ui->eMegaFolder->setText(QString::fromUtf8(tPath));
            delete [] tPath;
        }
        else
        {
            delete testNode;
            testNode = NULL;
            ui->eMegaFolder->setText(tr("/MEGAsync Imports"));
        }
    }
    else
    {
        ui->eMegaFolder->setText(tr("/MEGAsync Imports"));
    }

    MegaNode *p = testNode;
    while(p)
    {
        if(megaApi->isSynced(p))
        {
            ui->cDownload->setChecked(false);
            this->on_cDownload_clicked();
            delete p;
            break;
        }
        testNode = p;
        p = megaApi->getParentNode(testNode);
        delete testNode;
    }

	connect(linkProcessor, SIGNAL(onLinkInfoAvailable(int)), this, SLOT(onLinkInfoAvailable(int)));
	connect(linkProcessor, SIGNAL(onLinkInfoRequestFinish()), this, SLOT(onLinkInfoRequestFinish()));

	finished = false;
	linkProcessor->requestLinkInfo();
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
	return ui->eLocalFolder->text();
}

void ImportMegaLinksDialog::on_cDownload_clicked()
{
    if(ui->cImport->isChecked() && ui->cDownload->isChecked())
    {
        QString importFolder = ui->eMegaFolder->text();
        MegaNode *nImportFolder = megaApi->getNodeByPath(importFolder.toUtf8().constData());
        MegaNode *parent = nImportFolder;
        while(parent)
        {
            if(megaApi->isSynced(parent))
            {
                int result;
                if(linkProcessor->size()==1)
                {
                    result = QMessageBox::warning(this, tr("Warning"),
                        tr("You are about to import this file to a synced folder.\n"
                            "If you enable downloading, the file will be duplicated on your computer.\n"
                            "Are you sure?"), QMessageBox::Yes, QMessageBox::No);
                }
                else
                {
                    result = QMessageBox::warning(this, tr("Warning"),
                        tr("You are about to import these files to a synced folder.\n"
                            "If you enable downloading, the files will be duplicated on your computer.\n"
                            "Are you sure?"), QMessageBox::Yes, QMessageBox::No);
                }
                if(result != QMessageBox::Yes)
                {
                    ui->cDownload->setChecked(false);
                    delete parent;
                    return;
                }
                delete parent;
                break;
            }
            nImportFolder = parent;
            parent = megaApi->getParentNode(nImportFolder);
            delete nImportFolder;
        }
    }

	if(finished && (ui->cDownload->isChecked() || ui->cImport->isChecked()))
		ui->bOk->setEnabled(true);
	else
		ui->bOk->setEnabled(false);

	ui->bLocalFolder->setEnabled(ui->cDownload->isChecked());
	ui->eLocalFolder->setEnabled(ui->cDownload->isChecked());
}

void ImportMegaLinksDialog::on_cImport_clicked()
{
	if(finished && (ui->cDownload->isChecked() || ui->cImport->isChecked()))
		ui->bOk->setEnabled(true);
	else
		ui->bOk->setEnabled(false);

	ui->bMegaFolder->setEnabled(ui->cImport->isChecked());
	ui->eMegaFolder->setEnabled(ui->cImport->isChecked());
}

void ImportMegaLinksDialog::on_bLocalFolder_clicked()
{
    QString defaultPath = ui->eLocalFolder->text().trimmed();
    if(!defaultPath.size())
    {
        #ifdef WIN32
            #if QT_VERSION < 0x050000
                defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
            #else
                defaultPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
            #endif
        #else
            #if QT_VERSION < 0x050000
                defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
            #else
                defaultPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
            #endif
        #endif
    }
	QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
                                                      defaultPath,
													  QFileDialog::ShowDirsOnly
													  | QFileDialog::DontResolveSymlinks);
	if(path.length())
    {
        QDir dir(path);
        if(!dir.exists() && !dir.mkpath(QString::fromAscii("."))) return;

        QTemporaryFile test(path + QDir::separator());
        if(!test.open())
        {
            QMessageBox::critical(window(), tr("Error"), tr("You don't have write permissions in this local folder."));
            return;
        }

        ui->eLocalFolder->setText(path);
    }
}

void ImportMegaLinksDialog::on_bMegaFolder_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, false, this, false, false);
	int result = nodeSelector->exec();
	if(result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *selectedFolder = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if(!selectedFolder)
    {
        delete nodeSelector;
        return;
    }

    const char *fPath = megaApi->getNodePath(selectedFolder);
    if(!fPath)
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
    MegaNode *node = linkProcessor->getNode(id);

    int e = linkProcessor->getError(id);
    if(node && (e == MegaError::API_OK))
	{
        QString name = QString::fromUtf8(node->getName());
        if(!name.compare(QString::fromAscii("NO_KEY")) || !name.compare(QString::fromAscii("CRYPTO_ERROR")))
            item->setData(tr("Decryption error"), ImportListWidgetItem::WARNING, node->getSize());
		else
            item->setData(name, ImportListWidgetItem::CORRECT, node->getSize());
	}
	else
	{
        if((e != MegaError::API_OK) && (e != MegaError::API_ETOOMANY))
		{
			ImportListWidgetItem::linkstatus status = ImportListWidgetItem::FAILED;
            if(e == MegaError::API_ETEMPUNAVAIL)
				status = ImportListWidgetItem::WARNING;

            item->setData(QCoreApplication::translate("MegaError", MegaError::getErrorString(e)), status);
		}
		else
		{
            item->setData(tr("Not found"), ImportListWidgetItem::FAILED);
		}
	}
	item->updateGui();
}

void ImportMegaLinksDialog::onLinkInfoRequestFinish()
{
	finished = true;
	if(ui->cDownload->isChecked() || ui->cImport->isChecked())
		ui->bOk->setEnabled(true);
}

void ImportMegaLinksDialog::onLinkStateChanged(int id, int state)
{
    linkProcessor->setSelected(id, state);
}

void ImportMegaLinksDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        int totalImports = linkProcessor->getCurrentIndex();
        for(int i=0; i<totalImports; i++)
            this->onLinkInfoAvailable(i);
    }
    QDialog::changeEvent(event);
}
