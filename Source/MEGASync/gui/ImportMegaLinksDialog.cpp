#include "ImportMegaLinksDialog.h"
#include "ui_ImportMegaLinksDialog.h"
#include "gui/ImportListWidgetItem.h"
#include "gui/NodeSelector.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>

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

	QString defaultFolderPath;
	QFileInfo test(preferences->downloadFolder());
	if(!test.isDir())
	{
		#if QT_VERSION < 0x050000
            QDir defaultFolder(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + tr("/MEGAsync Downloads"));
		#else
            QDir defaultFolder(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0] + tr("/MEGAsync Downloads");
		#endif
        defaultFolder.mkpath(QString::fromAscii("."));
		defaultFolderPath = defaultFolder.absolutePath();
		defaultFolderPath = QDir::toNativeSeparators(defaultFolderPath);
	}
	else defaultFolderPath = preferences->downloadFolder();
	ui->eLocalFolder->setText(defaultFolderPath);

	Node *testNode = megaApi->getNodeByHandle(preferences->importFolder());
    if(testNode) ui->eMegaFolder->setText(QString::fromUtf8(megaApi->getNodePath(testNode)));
    else ui->eMegaFolder->setText(tr("/MEGAsync Imports"));

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
	QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
													  ui->eLocalFolder->text(),
													  QFileDialog::ShowDirsOnly
													  | QFileDialog::DontResolveSymlinks);
	if(path.length())
    {
        QDir dir(path);
        if(!dir.exists() && !dir.mkpath(QString::fromAscii("."))) return;
        ui->eLocalFolder->setText(path);
    }
}

void ImportMegaLinksDialog::on_bMegaFolder_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, this);
	nodeSelector->nodesReady();
	int result = nodeSelector->exec();
	if(result != QDialog::Accepted)
		return;

	handle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    ui->eMegaFolder->setText(QString::fromUtf8(megaApi->getNodePath(megaApi->getNodeByHandle(selectedMegaFolderHandle))));
}

void ImportMegaLinksDialog::onLinkInfoAvailable(int id)
{
	ImportListWidgetItem *item = (ImportListWidgetItem *)ui->linkList->itemWidget(ui->linkList->item(id));
    PublicNode *node = linkProcessor->getNode(id);
	item->setNode(node);

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

            item->setData(QString::fromUtf8(MegaError::getErrorString(e)), status);
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
