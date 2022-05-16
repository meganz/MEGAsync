#include "NodeSelector.h"
#include "ui_NodeSelector.h"
#include "ui_NewFolderDialog.h"

#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"

#include <QMessageBox>
#include <QPointer>
#include <QMenu>

using namespace mega;

// Human-friendly list of forbidden chars for New Remote Folder
static const QString forbidden(QString::fromLatin1("\\ / : \" * < > \? |"));
// Forbidden chars PCRE using a capture list: [\\/:"\*<>?|]
static const QRegularExpression forbiddenRx(QString::fromLatin1("[\\\\/:\"*<>\?|]"));
// Time to show the new remote folder input error
static int newFolderErrorDisplayTime = 10000; //10s in milliseconds

NodeSelector::NodeSelector(MegaApi *megaApi, int selectMode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeSelector),
    newFolderUi(new Ui::NewFolderDialog),
    newFolder(new QDialog(this))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowModality(Qt::ApplicationModal);

    this->megaApi = megaApi;
    this->model = NULL;
    folderIcon =  QIcon(QString::fromAscii("://images/small_folder.png"));
    selectedFolder = mega::INVALID_HANDLE;
    selectedItem = QModelIndex();
    this->selectMode = selectMode;
    delegateListener = new QTMegaRequestListener(megaApi, this);
    ui->cbAlwaysUploadToLocation->hide();
    ui->bOk->setDefault(true);

    if (selectMode == NodeSelector::STREAM_SELECT)
    {
        setWindowTitle(tr("Select items"));
        ui->label->setText(tr("Select just one file."));
        ui->bNewFolder->setVisible(false);
    }
    else if (selectMode == NodeSelector::DOWNLOAD_SELECT)
    {
        ui->bNewFolder->setVisible(false);
    }

    nodesReady();

    ui->tMegaFolders->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tMegaFolders, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    setupNewFolderDialog();
}

NodeSelector::~NodeSelector()
{
    delete delegateListener;
    delete ui;
    delete newFolder;
    delete newFolderUi;
    delete model;
}

void NodeSelector::nodesReady()
{
    if (!megaApi->isFilesystemAvailable())
    {
        ui->bOk->setEnabled(false);
        ui->bNewFolder->setEnabled(false);
        return;
    }

    model = new QMegaModel(megaApi);
    switch(selectMode)
    {
    case NodeSelector::UPLOAD_SELECT:
        model->setRequiredRights(MegaShare::ACCESS_READWRITE);
        model->showFiles(false);
        model->setDisableFolders(false);
        break;
    case NodeSelector::SYNC_SELECT:
        model->setRequiredRights(MegaShare::ACCESS_FULL);
        model->showFiles(false);
        model->setDisableFolders(false);
        break;
    case NodeSelector::DOWNLOAD_SELECT:
        model->setRequiredRights(MegaShare::ACCESS_READ);
        model->showFiles(true);
        model->setDisableFolders(false);
        break;
    case NodeSelector::STREAM_SELECT:
        model->setRequiredRights(MegaShare::ACCESS_READ);
        model->showFiles(true);
        model->setDisableFolders(false);
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

    if (selectMode == NodeSelector::STREAM_SELECT)
    {
        ui->tMegaFolders->expandToDepth(0);
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

MegaHandle NodeSelector::getSelectedFolderHandle()
{
    return selectedFolder;
}

void NodeSelector::setSelectedFolderHandle(MegaHandle selectedHandle)
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
        if (n && n->getHandle() == node->getHandle())
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
            if (n && n->getHandle() == node->getHandle())
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
    ui->bOk->setEnabled(true);

    if (e->getErrorCode() != MegaError::API_OK)
    {
        ui->tMegaFolders->setEnabled(true);
        QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"), tr("Error") + QString::fromUtf8(": ") + QCoreApplication::translate("MegaError", e->getErrorString()));
        return;
    }

    if(request->getType() == MegaRequest::TYPE_CREATE_FOLDER)
    {
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
    }
    else if (request->getType() == MegaRequest::TYPE_REMOVE || request->getType() == MegaRequest::TYPE_MOVE)
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            MegaNode *parent = model->getNode(selectedItem.parent());
            if (parent)
            {
                model->removeNode(selectedItem);
                setSelectedFolderHandle(parent->getHandle());
            }
        }
    }

    ui->tMegaFolders->setEnabled(true);
}

void NodeSelector::onCustomContextMenu(const QPoint &point)
{
    QMenu customMenu;

    MegaNode *node = megaApi->getNodeByHandle(selectedFolder);
    MegaNode *parent = megaApi->getParentNode(node);

    if (parent && node)
    {        
        int access = megaApi->getAccess(node);

        if (access == MegaShare::ACCESS_OWNER)
        {
            customMenu.addAction(tr("Get MEGA link"), this, SLOT(onGenMEGALinkClicked()));
        }

        if (access >= MegaShare::ACCESS_FULL)
        {
            customMenu.addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
        }
    }

    if (!customMenu.actions().isEmpty())
    {
        customMenu.exec(ui->tMegaFolders->mapToGlobal(point));
    }

    delete parent;
    delete node;
}

void NodeSelector::onDeleteClicked()
{
    MegaNode *node = megaApi->getNodeByHandle(selectedFolder);
    int access = megaApi->getAccess(node);
    if (!node || access < MegaShare::ACCESS_FULL)
    {
        delete node;
        return;
    }

    QPointer<NodeSelector> currentDialog = this;
    if (QMegaMessageBox::question(this,
                             QString::fromUtf8("MEGAsync"),
                             tr("Are you sure that you want to delete \"%1\"?")
                                .arg(QString::fromUtf8(node->getName())),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        if (!currentDialog)
        {
            delete node;
            return;
        }

        ui->tMegaFolders->setEnabled(false);
        ui->bNewFolder->setEnabled(false);
        ui->bOk->setEnabled(false);
        const char *name = node->getName();
        if (access == MegaShare::ACCESS_FULL
                || !strcmp(name, "NO_KEY")
                || !strcmp(name, "CRYPTO_ERROR")
                || !strcmp(name, "BLANK"))
        {
            megaApi->remove(node, delegateListener);
        }
        else
        {
            auto rubbish = ((MegaApplication*)qApp)->getRubbishNode();
            megaApi->moveNode(node, rubbish.get(), delegateListener);
        }
    }
    delete node;
}

void NodeSelector::onGenMEGALinkClicked()
{
    MegaNode *node = megaApi->getNodeByHandle(selectedFolder);
    if (!node || node->getType() == MegaNode::TYPE_ROOT
            || megaApi->getAccess(node) != MegaShare::ACCESS_OWNER)
    {
        delete node;
        return;
    }

    megaApi->exportNode(node);
    delete node;
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        newFolderUi->retranslateUi(newFolder);
        newFolderUi->errorLabel->setText(newFolderUi->errorLabel->text().arg(forbidden));
        nodesReady();
    }
    QDialog::changeEvent(event);
}

void NodeSelector::onSelectionChanged(QItemSelection, QItemSelection)
{
    if (ui->tMegaFolders->selectionModel()->selectedIndexes().size())
    {
        selectedItem = ui->tMegaFolders->selectionModel()->selectedIndexes().at(0);
        MegaNode *node = model->getNode(selectedItem);
        if (node)
        {
            selectedFolder =  node->getHandle();
            if (node->getType() == MegaNode::TYPE_FILE) {
                QString fileSizeText = QString::fromUtf8("File size: %L1 byte.").arg(node->getSize());
                ui->label->setText(fileSizeText);
            }
            else {
                ui->label->setText(QString());
            }
        }
        else
        {
            selectedFolder = mega::INVALID_HANDLE;
        }
    }
    else
    {
        selectedItem = QModelIndex();
        selectedFolder = mega::INVALID_HANDLE;
    }
}

void NodeSelector::on_bNewFolder_clicked()
{
    newFolderUi->errorLabel->hide();
    newFolderUi->textLabel->show();
    newFolderUi->lineEdit->clear();
    newFolderUi->lineEdit->setFocus();
    if (!newFolder->exec())
    {
        //dialog rejected, cancel New Folder operation
        return;
    }

    QString newFolderName = newFolderUi->lineEdit->text().trimmed();
    MegaNode *parent = megaApi->getNodeByHandle(selectedFolder);
    if (!parent)
    {
        auto rootNode = ((MegaApplication*)qApp)->getRootNode();
        if (rootNode)
        {
            parent = rootNode->copy();
        }
        if (!parent)
        {
            return;
        }
        selectedFolder = parent->getHandle();
        selectedItem = QModelIndex();
    }

    MegaNode *node = megaApi->getNodeByPath(newFolderName.toUtf8().constData(), parent);
    if (!node || node->isFile())
    {
        ui->bNewFolder->setEnabled(false);
        ui->bOk->setEnabled(false);
        ui->tMegaFolders->setEnabled(false);
        megaApi->createFolder(newFolderName.toUtf8().constData(), parent, delegateListener);
    }
    else
    {
        for (int i = 0; i < model->rowCount(selectedItem); i++)
        {
            QModelIndex row = model->index(i, 0, selectedItem);
            MegaNode *node = model->getNode(row);

            if (node && newFolderName.compare(QString::fromUtf8(node->getName())) == 0)
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

void NodeSelector::on_bOk_clicked()
{
    MegaNode *node = megaApi->getNodeByHandle(selectedFolder);
    if (!node)
    {
        reject();
        return;
    }

    int access = megaApi->getAccess(node);
    if ((selectMode == NodeSelector::UPLOAD_SELECT) && ((access < MegaShare::ACCESS_READWRITE)))
    {
        delete node;
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Read & Write or Full access rights to be able to upload to the selected folder."), QMessageBox::Ok);
        return;

    }
    else if ((selectMode == NodeSelector::SYNC_SELECT) && (access < MegaShare::ACCESS_FULL))
    {
        delete node;
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Full access right to be able to sync the selected folder."), QMessageBox::Ok);
        return;
    }
    else if ((selectMode == NodeSelector::STREAM_SELECT) && node->isFolder())
    {
        delete node;
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("Only files can be used for streaming."), QMessageBox::Ok);
        return;
    }

    if (selectMode == NodeSelector::SYNC_SELECT)
    {
        const char* path = megaApi->getNodePath(node);
        MegaNode *check = megaApi->getNodeByPath(path);
        delete [] path;
        if (!check)
        {
            delete node;
            QMegaMessageBox::warning(nullptr, tr("Warning"), tr("Invalid folder for synchronization.\n"
                                                         "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names."),
                                 QMessageBox::Ok);
            return;
        }
        delete check;
    }

    delete node;
    accept();
}

void NodeSelector::setupNewFolderDialog()
{
    // Initialize the NewFolder input Dialog
    newFolderUi->setupUi(newFolder);
    newFolder->setWindowFlags(newFolder->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    newFolderUi->errorLabel->setText(newFolderUi->errorLabel->text().arg(forbidden));
    // The dialog doesn't get resized on error
    newFolderUi->textLabel->setMinimumSize(newFolderUi->errorLabel->sizeHint());

    connect(newFolderUi->buttonBox, &QDialogButtonBox::rejected, newFolder, &QDialog::reject);
    QPushButton *okButton = newFolderUi->buttonBox->button(QDialogButtonBox::Ok);
    //only enabled when there's input, guards against empty folder name
    okButton->setEnabled(false);
    connect(newFolderUi->lineEdit, &QLineEdit::textChanged, this, [this, okButton]()
    {
        bool hasText = !newFolderUi->lineEdit->text().trimmed().isEmpty();
        okButton->setEnabled(hasText);
    });
    newFolderErrorTimer.setSingleShot(true);
    connect(&newFolderErrorTimer, &QTimer::timeout, this, [this]()
    {
        Utilities::animateFadeout(newFolderUi->errorLabel);
        // after animation is finished, hide the error label and show the original text
        // 700 magic number is how long Utilities::animateFadeout takes
        QTimer::singleShot(700, this, [this]()
        {
            newFolderUi->errorLabel->hide();
            newFolderUi->textLabel->show();
        });
    });
    connect(newFolderUi->buttonBox, &QDialogButtonBox::accepted, this, [this]
    {
        if(newFolderUi->lineEdit->text().trimmed().contains(forbiddenRx))
        {
            // show error label, dialog stays open
            newFolderUi->textLabel->hide();
            newFolderUi->errorLabel->show();
            Utilities::animateFadein(newFolderUi->errorLabel);
            newFolderErrorTimer.start(newFolderErrorDisplayTime); //(re)start timer
            newFolderUi->lineEdit->setFocus();
        }
        else
        {
            //dialog accepted, execute New Folder operation
            newFolder->accept();
        }
    });
}

bool NodeSelector::getDefaultUploadOption()
{
   return ui->cbAlwaysUploadToLocation->isChecked();
}
