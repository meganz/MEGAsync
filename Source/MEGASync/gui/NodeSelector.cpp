#include "NodeSelector.h"
#include "ui_NodeSelector.h"

#include <QMessageBox>
#include "sdk/SizeProcessor.h"
#include "utils/Utils.h"

NodeSelector::NodeSelector(MegaApi *megaApi, bool rootAllowed, bool sizeWarning, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeSelector)
{
    ui->setupUi(this);
    this->megaApi = megaApi;
    folderIcon =  QIcon(QString::fromAscii("://images/small_folder.png"));
    selectedFolder = UNDEF;
    selectedItem = NULL;
    this->rootAllowed = rootAllowed;
    this->sizeWarning = sizeWarning;
	delegateListener = new QTMegaRequestListener(this);
}

NodeSelector::~NodeSelector()
{
	delete delegateListener;
    delete ui;
}

void NodeSelector::nodesReady()
{
    ui->tMegaFolders->clear();
    Node *rootNode = megaApi->getRootNode();
    QTreeWidgetItem *root = new QTreeWidgetItem();
    root->setText(0, tr("Cloud Drive"));
    root->setIcon(0, folderIcon);
    root->setData(0, Qt::UserRole, (qulonglong)rootNode->nodehandle);
    addChildren(root, rootNode);
    ui->tMegaFolders->setIconSize(QSize(24, 24));
    ui->tMegaFolders->addTopLevelItem(root);
    ui->tMegaFolders->setCurrentItem(root);
    ui->tMegaFolders->expandToDepth(0);
}

long long NodeSelector::getSelectedFolderHandle()
{
    return selectedFolder;
}

void NodeSelector::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    cout << "Request finished!" << endl;
	if(e->getErrorCode() == MegaError::API_OK)
	{
		Node *node = megaApi->getNodeByHandle(request->getNodeHandle());
		QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString::fromUtf8(node->displayname()));
		item->setIcon(0, folderIcon);
		item->setData(0, Qt::UserRole, (qulonglong)node->nodehandle);
		selectedItem->addChild(item);
		selectedItem->sortChildren(0,  Qt::AscendingOrder);
		ui->tMegaFolders->setCurrentItem(item);
		ui->tMegaFolders->update();
	}
	ui->tMegaFolders->setEnabled(true);
}

void NodeSelector::addChildren(QTreeWidgetItem *parentItem, Node *parentNode)
{
    NodeList *children = megaApi->getChildren(parentNode);
    for(int i=0; i<children->size(); i++)
    {
        Node *node = children->get(i);
        if(node->type == FOLDERNODE)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, QString::fromUtf8(node->displayname()));
            item->setIcon(0, folderIcon);
            item->setData(0, Qt::UserRole, (qulonglong)node->nodehandle);
            parentItem->addChild(item);
            addChildren(item, node);
        }
    }
    delete children;
}


void NodeSelector::on_tMegaFolders_itemSelectionChanged()
{
    selectedItem =  ui->tMegaFolders->selectedItems().at(0);
    selectedFolder = selectedItem->data(0, Qt::UserRole).toULongLong();
}

void NodeSelector::on_bNewFolder_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New folder"),
                                         tr("Please, enter the new folder name:"), QLineEdit::Normal,
                                         QString::fromAscii(""), &ok);

    text = text.trimmed();
    if (ok && !text.isEmpty())
    {
        Node *parent = megaApi->getNodeByHandle(selectedFolder);
        Node *node = megaApi->getNodeByPath(text.toUtf8().constData(), parent);
        if(!node || (node->type==FILENODE))
        {
            ui->tMegaFolders->setEnabled(false);
			megaApi->createFolder(text.toUtf8().constData(), parent, delegateListener);
        }
        else
        {
            for(int i=0; i<selectedItem->childCount(); i++)
            {
                QTreeWidgetItem *item = selectedItem->child(i);
                if(item->text(0).compare(text)==0)
                {
                    ui->tMegaFolders->setCurrentItem(item);
                    ui->tMegaFolders->update();
                    break;
                }
            }
        }
    }
}

void NodeSelector::on_bOk_clicked()
{
    if(!rootAllowed && (selectedFolder == megaApi->getRootNode()->nodehandle))
    {
        QMessageBox::warning(this, tr("Error"), tr("The root folder can't be synced.\n"
                                                 "Please, select a subfolder"), QMessageBox::Ok);
        return;
    }

    if(sizeWarning)
    {
        SizeProcessor sizeProcessor;
        megaApi->processTree(megaApi->getNodeByHandle(selectedFolder), &sizeProcessor);
        long long totalSize = sizeProcessor.getTotalBytes();
        if(totalSize > 2147483648)
        {
            int res = QMessageBox::warning(this, tr("Warning"), tr("You have %1 in this folder.\n"
                                                         "Are you sure you want to sync it?")
                                                        .arg(Utils::getSizeString(totalSize)),
                                 QMessageBox::Yes, QMessageBox::No);
            if(res != QMessageBox::Yes)
            {
                return;
            }
        }
    }

    accept();
}
