#include "NodeSelector.h"
#include "ui_NodeSelector.h"

NodeSelector::NodeSelector(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeSelector)
{
    ui->setupUi(this);
    this->megaApi = megaApi;
    folderIcon =  QIcon(":/images/folder.ico");
    selectedFolder = UNDEF;
    selectedItem = NULL;
}

NodeSelector::~NodeSelector()
{
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
    ui->tMegaFolders->addTopLevelItem(root);
    ui->tMegaFolders->setCurrentItem(root);
    ui->tMegaFolders->expandAll();
}

long long NodeSelector::getSelectedFolderHandle()
{
    return selectedFolder;
}


bool NodeSelector::event(QEvent *event)
{
    if(event->type() != QEvent::User)
        return QDialog::event(event);

    if(error->getErrorCode() == MegaError::API_OK)
    {
        Node *node = megaApi->getNodeByHandle(request->getNodeHandle());
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, node->displayname());
        item->setIcon(0, folderIcon);
        item->setData(0, Qt::UserRole, (qulonglong)node->nodehandle);
        selectedItem->addChild(item);
        selectedItem->sortChildren(0,  Qt::AscendingOrder);
        ui->tMegaFolders->setCurrentItem(item);
        ui->tMegaFolders->update();
    }
    ui->tMegaFolders->setEnabled(true);
    delete request;
    delete error;
}
void NodeSelector::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    cout << "Request finished!" << endl;
    this->request = request->copy();
    this->error = e->copy();
    QApplication::postEvent(this, new QEvent(QEvent::User));
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
            item->setText(0, node->displayname());
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
                                         "", &ok);

    text = text.trimmed();
    if (ok && !text.isEmpty())
    {
        Node *parent = megaApi->getNodeByHandle(selectedFolder);
        Node *node = megaApi->getNodeByPath(text.toUtf8().constData(), parent);
        if(!node || (node->type==FILENODE))
        {
            ui->tMegaFolders->setEnabled(false);
            megaApi->createFolder(text.toUtf8().constData(), parent, this);
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
