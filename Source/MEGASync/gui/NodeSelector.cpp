#include "NodeSelector.h"
#include "ui_NodeSelector.h"

#include <QMessageBox>
#include <QPointer>
#include "control/Utilities.h"


using namespace mega;

NodeSelector::NodeSelector(MegaApi *megaApi, int selectMode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeSelector)
{
    ui->setupUi(this);
    this->megaApi = megaApi;
    folderIcon =  QIcon(QString::fromAscii("://images/small_folder.png"));
    selectedFolder = mega::INVALID_HANDLE;
    selectedItem = QModelIndex();
    this->selectMode = selectMode;
    delegateListener = new QTMegaRequestListener(megaApi, this);
    ui->cbAlwaysUploadToLocation->hide();
    ui->bOk->setDefault(true);

    nodesReady();

#ifdef 0
    QT_TR_NOOP("Are you sure that you want to delete \"%1\"?");
#endif
}

NodeSelector::~NodeSelector()
{
    delete delegateListener;
    delete ui;
}

void NodeSelector::nodesReady()
{
    MegaNode *rootNode = megaApi->getRootNode();
    if (!rootNode)
    {
        return;
    }

    model = new QMegaModel(megaApi);
    switch(selectMode)
    {
    case NodeSelector::UPLOAD_SELECT:
        model->setRequiredRights(MegaShare::ACCESS_READWRITE);
        model->showFiles(false);
        break;
    case NodeSelector::SYNC_SELECT:
        model->setRequiredRights(MegaShare::ACCESS_FULL);
        model->showFiles(false);
        break;
    case NodeSelector::DOWNLOAD_SELECT:
        model->setRequiredRights(MegaShare::ACCESS_READ);
        model->showFiles(true);
        break;
    }

    ui->tMegaFolders->setModel(model);
    connect(ui->tMegaFolders->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));

    ui->tMegaFolders->collapseAll();
    ui->tMegaFolders->header()->close();
//Disable animation for OS X due to problems showing the tree icons
#ifdef __APPLE__
    ui->tMegaFolders->setAnimated(false);
#endif

    QModelIndex defaultSelection = model->index(0, 0);
    ui->tMegaFolders->selectionModel()->select(defaultSelection, QItemSelectionModel::ClearAndSelect);
    ui->tMegaFolders->selectionModel()->setCurrentIndex(defaultSelection, QItemSelectionModel::ClearAndSelect);
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
    if (!node)
    {
        return;
    }

    QList<MegaNode *> list;
    while (node)
    {
        list.append(node);
        node = megaApi->getParentNode(node);
    }

    if (!list.size())
    {
        return;
    }

    int index = list.size() - 1;
    QModelIndex modelIndex;
    QModelIndex parentModelIndex;
    node = list.at(index);

    for (int i = 0; i < model->rowCount(); i++)
    {
        QModelIndex tmp = model->index(i, 0);
        MegaNode *n = model->getNode(tmp);
        if (n->getHandle() == node->getHandle())
        {
            node = NULL;
            parentModelIndex = modelIndex;
            modelIndex = tmp;
            index--;
            ui->tMegaFolders->expand(parentModelIndex);
            break;
        }
    }

    if (node)
    {
        for (int k = 0; k < list.size(); k++)
        {
            delete list.at(k);
        }
        ui->tMegaFolders->collapseAll();
        return;
    }

    while (index >= 0)
    {
        node = list.at(index);
        for (int j = 0; j < model->rowCount(modelIndex); j++)
        {
            QModelIndex tmp = model->index(j, 0, modelIndex);
            MegaNode *n = model->getNode(tmp);
            if (n->getHandle() == node->getHandle())
            {
                node = NULL;
                parentModelIndex = modelIndex;
                modelIndex = tmp;
                index--;
                ui->tMegaFolders->expand(parentModelIndex);
                break;
            }
        }

        if (node)
        {
            for (int k = 0; k < list.size(); k++)
            {
                delete list.at(k);
            }
            ui->tMegaFolders->collapseAll();
            return;
        }
    }

    for (int k = 0; k < list.size(); k++)
    {
        delete list.at(k);
    }

    ui->tMegaFolders->selectionModel()->setCurrentIndex(modelIndex, QItemSelectionModel::ClearAndSelect);
    ui->tMegaFolders->selectionModel()->select(modelIndex, QItemSelectionModel::ClearAndSelect);
}

void NodeSelector::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    ui->bNewFolder->setEnabled(true);
    if (e->getErrorCode() == MegaError::API_OK)
    {
        MegaNode *node = megaApi->getNodeByHandle(request->getNodeHandle());
        if (node)
        {
            QModelIndex row = model->insertNode(node, selectedItem);
            setSelectedFolderHandle(node->getHandle());
            ui->tMegaFolders->selectionModel()->select(row, QItemSelectionModel::ClearAndSelect);
            ui->tMegaFolders->selectionModel()->setCurrentIndex(row, QItemSelectionModel::ClearAndSelect);
        }
    }
    else
    {
        QMessageBox::critical(this, QString::fromUtf8("MEGAsync"), tr("Error") + QString::fromUtf8(": ") + QCoreApplication::translate("MegaError", e->getErrorString()));
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

void NodeSelector::onSelectionChanged(QItemSelection, QItemSelection)
{
    selectedItem = ui->tMegaFolders->selectionModel()->selectedIndexes().at(0);
    selectedFolder =  model->getNode(selectedItem)->getHandle();
}

void NodeSelector::on_bNewFolder_clicked()
{
    QPointer<QInputDialog> id = new QInputDialog(this);
    id->setWindowTitle(tr("New folder"));
    id->setLabelText(tr("Enter the new folder name:"));
    int result = id->exec();

    if (!id || !result)
    {
        delete id;
        return;
    }

    QString text = id->textValue();
    text = text.trimmed();
    if (!text.isEmpty())
    {
        MegaNode *parent = megaApi->getNodeByHandle(selectedFolder);
        MegaNode *node = megaApi->getNodeByPath(text.toUtf8().constData(), parent);
        if (!node || node->isFile())
        {
            ui->bNewFolder->setEnabled(false);
            ui->tMegaFolders->setEnabled(false);
            megaApi->createFolder(text.toUtf8().constData(), parent, delegateListener);
        }
        else
        {
            for (int i = 0; i < model->rowCount(selectedItem); i++)
            {
                QModelIndex row = model->index(i, 0, selectedItem);
                MegaNode *node = model->getNode(row);

                if (text.compare(QString::fromUtf8(node->getName())) == 0)
                {
                    setSelectedFolderHandle(node->getHandle());
                    ui->tMegaFolders->selectionModel()->select(row, QItemSelectionModel::ClearAndSelect);
                    ui->tMegaFolders->selectionModel()->setCurrentIndex(row, QItemSelectionModel::ClearAndSelect);
                    break;
                }
            }
        }
        delete parent;
        delete node;
    }
    else
    {
        QMessageBox::critical(this, QString::fromUtf8("MEGAsync"), tr("Please enter a valid folder name"));
    }
    delete id;
}

void NodeSelector::on_bOk_clicked()
{
    MegaNode *node = megaApi->getNodeByHandle(selectedFolder);
    int access = megaApi->getAccess(node);
    if ((selectMode == NodeSelector::UPLOAD_SELECT) && ((access < MegaShare::ACCESS_READWRITE)))
    {
            QMessageBox::warning(this, tr("Error"), tr("You need Read & Write or Full access rights to be able to upload to the selected folder."), QMessageBox::Ok);
            delete node;
            return;

    }
    else if ((selectMode == NodeSelector::SYNC_SELECT) && (access < MegaShare::ACCESS_FULL))
    {
        QMessageBox::warning(this, tr("Error"), tr("You need Full access right to be able to sync the selected folder."), QMessageBox::Ok);
        delete node;
        return;
    }

    const char* path = megaApi->getNodePath(node);
    MegaNode *check = megaApi->getNodeByPath(path);
    delete [] path;
    delete node;
    if (!check)
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
