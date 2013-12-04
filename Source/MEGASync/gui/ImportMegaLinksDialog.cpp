#include "ImportMegaLinksDialog.h"
#include "ui_ImportMegaLinksDialog.h"
#include "gui/ImportListWidgetItem.h"
#include "gui/NodeSelector.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>

ImportMegaLinksDialog::ImportMegaLinksDialog(MegaApi *megaApi, LinkProcessor *processor, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ImportMegaLinksDialog)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_QuitOnClose, false);

	this->megaApi = megaApi;
	this->linkProcessor = processor;

	for(int i=0; i<linkProcessor->size(); i++)
	{
		ImportListWidgetItem *customItem = new ImportListWidgetItem(linkProcessor->getLink(i), ui->linkList);
		QListWidgetItem *item = new QListWidgetItem(ui->linkList);
		ui->linkList->addItem(item);
		item->setSizeHint(customItem->size());
		ui->linkList->setItemWidget(item, customItem);
	}

	if(linkProcessor->size()<=8)
	{
		ui->linkList->setMinimumHeight(ui->linkList->minimumHeight()+32*(linkProcessor->size()-1));
		this->setMinimumHeight(this->minimumHeight()+32*(linkProcessor->size()-1));
	}
	else
	{
		ui->linkList->setMinimumHeight(ui->linkList->minimumHeight()+32*7);
		this->setMinimumHeight(this->minimumHeight()+32*7);
	}

	ui->linkList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setMaximumHeight(this->minimumHeight());

	#if QT_VERSION < 0x050000
		QDir defaultFolder(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/MEGAsync Downloads");
	#else
		QDir defaultFolder(QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation)[0] + "/MEGAsync Downloads");
	#endif
	defaultFolder.mkpath(".");
	QString defaultFolderPath = defaultFolder.absolutePath();
#ifdef WIN32
   defaultFolderPath = defaultFolderPath.replace("/","\\");
#endif
	ui->eLocalFolder->setText(defaultFolderPath);
	ui->eMegaFolder->setText("/MEGAsync Imports");

	connect(linkProcessor, SIGNAL(onLinkInfoAvailable(int)), this, SLOT(onLinkInfoAvailable(int)));
	connect(linkProcessor, SIGNAL(onLinkInfoRequestFinish()), this, SLOT(onLinkInfoRequestFinish()));

	finished = false;
	linkProcessor->requestLinkInfo();
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
	if(finished && (ui->cDownload->isChecked() || ui->cImport->isChecked()))
		ui->bOk->setEnabled(true);
	else
		ui->bOk->setEnabled(false);

	ui->bLocalFolder->setEnabled(ui->cDownload->isChecked());
}

void ImportMegaLinksDialog::on_cImport_clicked()
{
	if(finished && (ui->cDownload->isChecked() || ui->cImport->isChecked()))
		ui->bOk->setEnabled(true);
	else
		ui->bOk->setEnabled(false);

	ui->bMegaFolder->setEnabled(ui->cImport->isChecked());
}

void ImportMegaLinksDialog::on_bLocalFolder_clicked()
{
	QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
													  ui->eLocalFolder->text(),
													  QFileDialog::ShowDirsOnly
													  | QFileDialog::DontResolveSymlinks);
	if(path.length())
		ui->eLocalFolder->setText(path);
}

void ImportMegaLinksDialog::on_bMegaFolder_clicked()
{
	NodeSelector *nodeSelector = new NodeSelector(megaApi, this);
	nodeSelector->nodesReady();
	int result = nodeSelector->exec();
	if(result != QDialog::Accepted)
		return;

	handle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
	ui->eMegaFolder->setText(megaApi->getNodePath(megaApi->getNodeByHandle(selectedMegaFolderHandle)));
}

void ImportMegaLinksDialog::onLinkInfoAvailable(int id)
{
	ImportListWidgetItem *item = (ImportListWidgetItem *)ui->linkList->itemWidget(ui->linkList->item(id));
	Node *node = linkProcessor->getNode(id);
	item->setNode(node);

	if(node && (linkProcessor->getError(id) == MegaError::API_OK))
	{
		QString name = QString::fromUtf8(node->displayname());
		if(!name.compare("NO_KEY") || !name.compare("CRYPTO_ERROR"))
			item->setData("Decryption error", ImportListWidgetItem::WARNING);
		else
			item->setData(name, ImportListWidgetItem::CORRECT);
	}
	else
	{
		if(linkProcessor->getError(id) != MegaError::API_OK)
		{
			ImportListWidgetItem::linkstatus status = ImportListWidgetItem::FAILED;
			if(linkProcessor->getError(id) == MegaError::API_ETEMPUNAVAIL)
				status = ImportListWidgetItem::WARNING;

			item->setData(MegaError::getErrorString(linkProcessor->getError(id)), status);
		}
		else
		{
			item->setData("Not found", ImportListWidgetItem::FAILED);
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
