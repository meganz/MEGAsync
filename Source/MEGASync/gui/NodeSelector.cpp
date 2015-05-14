#include "NodeSelector.h"
#include "ui_NodeSelector.h"

#include <QMessageBox>
#include "control/Utilities.h"


using namespace mega;

NodeSelector::NodeSelector(MegaApi *megaApi, bool rootAllowed, bool sizeWarning, int selectMode, QWidget *parent, bool showInshares) :
    QDialog(parent),
    ui(new Ui::NodeSelector)
{
    ui->setupUi(this);
    this->megaApi = megaApi;
    folderIcon =  QIcon(QString::fromAscii("://images/small_folder.png"));
    selectedFolder = mega::INVALID_HANDLE;
    selectedItem = NULL;
    this->rootAllowed = rootAllowed;
    this->sizeWarning = sizeWarning;
    this->selectMode = selectMode;
    this->showInshares = showInshares;
    delegateListener = new QTMegaRequestListener(megaApi, this);
    ui->cbAlwaysUploadToLocation->hide();
    ui->bOk->setDefault(true);

    nodesReady();
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

    if(showInshares)
    {
        MegaUserList *contacts = megaApi->getContacts();
        for(int i=0; i < contacts->size(); i++)
        {
            MegaUser *contact = contacts->get(i);
            MegaNodeList *folders = megaApi->getInShares(contact);
            for(int j=0; j < folders->size(); j++)
            {
                MegaNode *folder = folders->get(j);
                if(megaApi->getAccess(folder) == MegaShare::ACCESS_UNKNOWN)
                {
                    continue;
                }
                QTreeWidgetItem *item = new QTreeWidgetItem();
                item->setText(0, QString::fromUtf8("%1 (%2)").arg(QString::fromUtf8(folder->getName())).arg(QString::fromUtf8(contact->getEmail())));
                item->setIcon(0, folderIcon);
                item->setData(0, Qt::UserRole, (qulonglong)folder->getHandle());

                int access = megaApi->getAccess(folder);
                if((selectMode == NodeSelector::UPLOAD_SELECT) && ((access != MegaShare::ACCESS_FULL) && (access != MegaShare::ACCESS_READWRITE))
                        || ((selectMode == NodeSelector::SYNC_SELECT) && (access != MegaShare::ACCESS_FULL)))
                {
                    QBrush b (QColor(170,170,170, 127));
                    item->setForeground(0,b);
                }
                addChildren(item, folder);
                ui->tMegaFolders->addTopLevelItem(item);
            }
            delete folders;
        }
        delete contacts;
    }
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    ui->cbAlwaysUploadToLocation->setVisible(show);
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

long long NodeSelector::getSelectedFolderHandle()
{
    return selectedFolder;
}

void NodeSelector::setSelectedFolderHandle(long long selectedHandle)
{
    MegaNode *node = megaApi->getNodeByHandle(selectedHandle);
    if(!node)
    {
        return;
    }

    QList<MegaNode *> list;
    while(node)
    {
        list.append(node);
        node = megaApi->getParentNode(node);
    }

    if(!list.size())
    {
        return;
    }

    QTreeWidgetItem *item = NULL;
    int index = list.size() - 1;
    node = list.at(index);

    for(int i = 0; i < ui->tMegaFolders->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *tmp = ui->tMegaFolders->topLevelItem(i);
        if(tmp->data(0, Qt::UserRole).toLongLong() == node->getHandle())
        {
            node = NULL;
            item = tmp;
            index--;
            break;
        }
    }

    if(node)
    {
        for(int k = 0; k < list.size(); k++)
            delete list.at(k);
        return;
    }

    while(index >= 0)
    {
        node = list.at(index);
        for(int j = 0; j < item->childCount(); j++)
        {
            QTreeWidgetItem *tmp = item->child(j);
            if(tmp->data(0, Qt::UserRole).toLongLong() == node->getHandle())
            {
                node = NULL;
                item = tmp;
                index--;
                break;
            }
        }

        if(node)
        {
            for(int k = 0; k < list.size(); k++)
                delete list.at(k);
            return;
        }
    }

    for(int k = 0; k < list.size(); k++)
        delete list.at(k);

    ui->tMegaFolders->setCurrentItem(item);
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
    MegaNodeList *children = megaApi->getChildren(parentNode);
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

        }else if(selectMode == NodeSelector::DOWNLOAD_SELECT)
        {
            QIcon icon;
            icon.addFile(Utilities::getExtensionPixmapSmall(QString::fromUtf8(node->getName())), QSize(), QIcon::Normal, QIcon::Off);
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, QString::fromUtf8(node->getName()));
            item->setIcon(0, icon);
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
    int access = megaApi->getAccess(node);
    if((selectMode == NodeSelector::UPLOAD_SELECT) && ((access != MegaShare::ACCESS_FULL) && (access != MegaShare::ACCESS_READWRITE)))
    {
            QMessageBox::warning(this, tr("Error"), tr("You need Read & Write or Full access rights to be able to upload to the selected folder."), QMessageBox::Ok);
            delete node;
            return;

    }else if((selectMode == NodeSelector::SYNC_SELECT) && (access != MegaShare::ACCESS_FULL))
    {
        QMessageBox::warning(this, tr("Error"), tr("You need Full access right to be able to sync the selected folder."), QMessageBox::Ok);
        delete node;
        return;
    }

    const char* path = megaApi->getNodePath(node);
    MegaNode *check = megaApi->getNodeByPath(path);
    delete [] path;
    delete node;
    if(!check)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Invalid folder for synchronization.\n"
                                                     "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names."),
                             QMessageBox::Ok);
        return;
    }
    delete check;
    accept();
}

bool NodeSelector::getDefaultUploadOption()
{
   return ui->cbAlwaysUploadToLocation->isChecked();
}
