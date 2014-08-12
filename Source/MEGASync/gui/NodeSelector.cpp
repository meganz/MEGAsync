#include "NodeSelector.h"
#include "ui_NodeSelector.h"

#include <QMessageBox>
#include "control/Utilities.h"

using namespace mega;

NodeSelector::NodeSelector(MegaApi *megaApi, bool rootAllowed, bool sizeWarning, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeSelector)
{
    ui->setupUi(this);
    this->megaApi = megaApi;
    folderIcon =  QIcon(QString::fromAscii("://images/small_folder.png"));
    selectedFolder = mega::UNDEF;
    selectedItem = NULL;
    this->rootAllowed = rootAllowed;
    this->sizeWarning = sizeWarning;
    delegateListener = new QTMegaRequestListener(megaApi, this);
    ui->bOk->setDefault(true);
}

NodeSelector::~NodeSelector()
{
	delete delegateListener;
    delete ui;
}

void NodeSelector::nodesReady()
{
    MegaNode *rootNode = megaApi->getRootNode();
    if(!rootNode) return;

    ui->tMegaFolders->clear();
    QTreeWidgetItem *root = new QTreeWidgetItem();
    root->setText(0, tr("Cloud Drive"));
    root->setIcon(0, folderIcon);
    root->setData(0, Qt::UserRole, (qulonglong)rootNode->getHandle());
    addChildren(root, rootNode);
    ui->tMegaFolders->setIconSize(QSize(24, 24));
    ui->tMegaFolders->addTopLevelItem(root);
    ui->tMegaFolders->setCurrentItem(root);
    ui->tMegaFolders->expandToDepth(0);
    selectedItem = root;
    selectedFolder = selectedItem->data(0, Qt::UserRole).toULongLong();
    delete rootNode;
}

long long NodeSelector::getSelectedFolderHandle()
{
    return selectedFolder;
}

void NodeSelector::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    ui->bNewFolder->setEnabled(true);
	if(e->getErrorCode() == MegaError::API_OK)
	{
        MegaNode *node = megaApi->getNodeByHandle(request->getNodeHandle());
        if(node)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, QString::fromUtf8(node->getName()));
            item->setIcon(0, folderIcon);
            item->setData(0, Qt::UserRole, (qulonglong)node->getHandle());
            selectedItem->addChild(item);
            selectedItem->sortChildren(0,  Qt::AscendingOrder);
            ui->tMegaFolders->setCurrentItem(item);
            ui->tMegaFolders->update();
            delete node;
        }
	}
    ui->tMegaFolders->setEnabled(true);
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        nodesReady();
    }
    QDialog::changeEvent(event);
}

void NodeSelector::addChildren(QTreeWidgetItem *parentItem, MegaNode *parentNode)
{
    NodeList *children = megaApi->getChildren(parentNode);
    for(int i=0; i<children->size(); i++)
    {
        MegaNode *node = children->get(i);
        if(node->getType() == MegaNode::TYPE_FOLDER)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, QString::fromUtf8(node->getName()));
            item->setIcon(0, folderIcon);
            item->setData(0, Qt::UserRole, (qulonglong)node->getHandle());
            parentItem->addChild(item);
            addChildren(item, node);
        }
    }
    delete children;
}


void NodeSelector::on_tMegaFolders_itemSelectionChanged()
{
    if(!ui->tMegaFolders->selectedItems().size()) return;
    selectedItem =  ui->tMegaFolders->selectedItems().at(0);
    selectedFolder = selectedItem->data(0, Qt::UserRole).toULongLong();
}

void NodeSelector::on_bNewFolder_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New folder"),
                                         tr("Enter the new folder name:"), QLineEdit::Normal,
                                         QString::fromAscii(""), &ok);

    text = text.trimmed();
    if (ok && !text.isEmpty())
    {
        MegaNode *parent = megaApi->getNodeByHandle(selectedFolder);
        MegaNode *node = megaApi->getNodeByPath(text.toUtf8().constData(), parent);
        if(!node || (node->isFile()))
        {
            ui->tMegaFolders->setEnabled(false);
			megaApi->createFolder(text.toUtf8().constData(), parent, delegateListener);
            ui->bNewFolder->setEnabled(false);
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
        delete parent;
        delete node;
    }
}

void NodeSelector::on_bOk_clicked()
{
    MegaNode *rootNode = megaApi->getRootNode();
    if(!rootNode) return;

    if(!rootAllowed && (selectedFolder == rootNode->getHandle()))
    {
        QMessageBox::warning(this, tr("Error"), tr("The root folder can't be synced.\n"
                                                 "Please, select a subfolder."), QMessageBox::Ok);
        delete rootNode;
        return;
    }
    delete rootNode;

    MegaNode *node = megaApi->getNodeByHandle(selectedFolder);
    MegaNode *check = megaApi->getNodeByPath(megaApi->getNodePath(node));
    delete node;
    if(!check)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Invalid folder for synchronization.\n"
                                                     "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names."),
                             QMessageBox::Ok);
        return;
    }

    if(sizeWarning)
    {
        long long totalSize = megaApi->getSize(megaApi->getNodeByHandle(selectedFolder));
        if(totalSize > 2147483648)
        {
            int res = QMessageBox::warning(this, tr("Warning"), tr("You have %1 in this folder.\n"
                                                         "Are you sure you want to sync it?")
                                                        .arg(Utilities::getSizeString(totalSize)),
                                 QMessageBox::Yes, QMessageBox::No);
            if(res != QMessageBox::Yes)
            {
                return;
            }
        }
    }

    accept();
}
