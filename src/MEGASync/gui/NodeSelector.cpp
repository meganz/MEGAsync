#include "NodeSelector.h"
#include "ui_NodeSelector.h"
#include "ui_NewFolderDialog.h"

#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"

#include <QMessageBox>
#include <QPointer>
#include <QMenu>

// Human-friendly list of forbidden chars for New Remote Folder
static const QString FORBIDDEN(QLatin1String("\\ / : \" * < > \? |"));
// Forbidden chars PCRE using a capture list: [\\/:"\*<>?|]
static const QRegularExpression FORBIDDEN_REGEXP(QLatin1String("[\\\\/:\"*<>\?|]"));
// Time to show the new remote folder input error
static constexpr int NEW_FOLDER_DISPLAY_TIME_MS = 10000; //10s in milliseconds

NodeSelector::NodeSelector(mega::MegaApi*megaApi, SelectMode selectMode, QWidget *parent) :
    QDialog(parent),
    mDelegateListener (new mega::QTMegaRequestListener(megaApi, this)),
    mNodeSelectorUi(new Ui::NodeSelector),
    mNewFolderUi(new Ui::NewFolderDialog),
    mNewFolderDialog(new QDialog(this)),
    mMegaApi (megaApi),
    mSelectedFolder (mega::INVALID_HANDLE),
    mSelectedItemIndex (QModelIndex()),
    mSelectMode (selectMode),
    mRemoteTreeModel (nullptr),
    mMyBackupsRootDirHandle (mega::INVALID_HANDLE)
{
    mNodeSelectorUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::ApplicationModal);

    mNodeSelectorUi->cbAlwaysUploadToLocation->hide();
    mNodeSelectorUi->bOk->setDefault(true);
    mNodeSelectorUi->bOk->setEnabled(false);

    if (selectMode == SelectMode::STREAM_SELECT)
    {
        setWindowTitle(tr("Select items"));
        mNodeSelectorUi->label->setText(tr("Select just one file."));
        mNodeSelectorUi->bNewFolder->setVisible(false);
        mNodeSelectorUi->bNewFolder->setEnabled(false);
    }
    else if (selectMode == SelectMode::DOWNLOAD_SELECT)
    {
        mNodeSelectorUi->bNewFolder->setVisible(false);
    }

    nodesReady();

    mNodeSelectorUi->tMegaFolders->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mNodeSelectorUi->tMegaFolders, &QTreeView::customContextMenuRequested,
            this, &NodeSelector::onCustomContextMenu);

    connect(&mSyncController, &SyncController::myBackupsHandle,
            this, &NodeSelector::onMyBackupsRootDir);
    mSyncController.getMyBackupsHandle();

    setupNewFolderDialog();
}

NodeSelector::~NodeSelector()
{
    delete mNodeSelectorUi;
    delete mNewFolderUi;
}

void NodeSelector::nodesReady()
{
    if (!mMegaApi->isFilesystemAvailable())
    {
        mNodeSelectorUi->bOk->setEnabled(false);
        mNodeSelectorUi->bNewFolder->setEnabled(false);
        return;
    }

    auto accessRights (mega::MegaShare::ACCESS_UNKNOWN);
    bool showFiles (false);
    bool setDisableFolders (false);
    bool setDisableBackups (false);

    switch (mSelectMode)
    {
        case SelectMode::UPLOAD_SELECT:
        {
            accessRights = mega::MegaShare::ACCESS_READWRITE;
            showFiles = false;
            setDisableFolders = false;
            setDisableBackups = true;
            break;
        }
        case SelectMode::SYNC_SELECT:
        {
            accessRights = mega::MegaShare::ACCESS_FULL;
            showFiles = false;
            setDisableFolders = false;
            setDisableBackups = true;
            break;
        }
        case SelectMode::DOWNLOAD_SELECT:
        {
            accessRights = mega::MegaShare::ACCESS_READ;
            showFiles = true;
            setDisableFolders = false;
            setDisableBackups = false;
            break;
        }
        case SelectMode::STREAM_SELECT:
        {
            accessRights = mega::MegaShare::ACCESS_READ;
            showFiles = true;
            setDisableFolders = false;
            setDisableBackups = false;
            break;
        }
    }

    mRemoteTreeModel.reset(new QMegaModel(mMegaApi));
    mRemoteTreeModel->setRequiredRights(accessRights);
    mRemoteTreeModel->showFiles(showFiles);
    mRemoteTreeModel->setDisableFolders(setDisableFolders);
    mRemoteTreeModel->setDisableBackups(setDisableBackups);
    mNodeSelectorUi->tMegaFolders->setModel(mRemoteTreeModel.get());

    QItemSelectionModel* selectionModel (mNodeSelectorUi->tMegaFolders->selectionModel());
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &NodeSelector::onSelectionChanged);

    mNodeSelectorUi->tMegaFolders->collapseAll();
    mNodeSelectorUi->tMegaFolders->header()->close();
//Disable animation for OS X due to problems showing the tree icons
#ifdef __APPLE__
    mNodeSelectorUi->tMegaFolders->setAnimated(false);
#endif

    QModelIndex defaultSelection (mRemoteTreeModel->index(0, 0));
    selectionModel->select(defaultSelection, QItemSelectionModel::ClearAndSelect);
    selectionModel->setCurrentIndex(defaultSelection, QItemSelectionModel::ClearAndSelect);

    if (mSelectMode == SelectMode::STREAM_SELECT)
    {
        mNodeSelectorUi->tMegaFolders->expandToDepth(0);
    }
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    mNodeSelectorUi->cbAlwaysUploadToLocation->setVisible(show);
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    mNodeSelectorUi->cbAlwaysUploadToLocation->setChecked(value);
}

mega::MegaHandle NodeSelector::getSelectedFolderHandle()
{
    return mSelectedFolder;
}

void NodeSelector::setSelectedFolderHandle(mega::MegaHandle selectedHandle)
{
    mega::MegaNode *node = mMegaApi->getNodeByHandle(selectedHandle);
    if (!node)
    {
        return;
    }

    QList<mega::MegaNode *> list;
    do
    {
        list.append(node);
        node = mMegaApi->getParentNode(node);
    }
    while (node);

    int index = list.size() - 1;
    QModelIndex modelIndex;
    QModelIndex parentModelIndex;
    node = list.at(index);

    for (int i = 0; i < mRemoteTreeModel->rowCount(); i++)
    {
        QModelIndex tmp = mRemoteTreeModel->index(i, 0);
        auto n (mRemoteTreeModel->getNode(tmp));
        if (n && n->getHandle() == node->getHandle())
        {
            node = nullptr;
            parentModelIndex = modelIndex;
            modelIndex = tmp;
            index--;
            mNodeSelectorUi->tMegaFolders->expand(parentModelIndex);
            break;
        }
    }

    if (node)
    {
        qDeleteAll(list);
        mNodeSelectorUi->tMegaFolders->collapseAll();
        return;
    }

    while (index >= 0)
    {
        node = list.at(index);
        for (int j = 0; j < mRemoteTreeModel->rowCount(modelIndex); j++)
        {
            QModelIndex tmp = mRemoteTreeModel->index(j, 0, modelIndex);
            auto n (mRemoteTreeModel->getNode(tmp));
            if (n && n->getHandle() == node->getHandle())
            {
                node = nullptr;
                parentModelIndex = modelIndex;
                modelIndex = tmp;
                index--;
                mNodeSelectorUi->tMegaFolders->expand(parentModelIndex);
                break;
            }
        }

        if (node)
        {
            qDeleteAll(list);
            mNodeSelectorUi->tMegaFolders->collapseAll();
            return;
        }
    }

    qDeleteAll(list);

    mNodeSelectorUi->tMegaFolders->selectionModel()->setCurrentIndex(modelIndex, QItemSelectionModel::ClearAndSelect);
    mNodeSelectorUi->tMegaFolders->selectionModel()->select(modelIndex, QItemSelectionModel::ClearAndSelect);
}

void NodeSelector::onRequestFinish(mega::MegaApi*, mega::MegaRequest* request, mega::MegaError* e)
{
    mNodeSelectorUi->bNewFolder->setEnabled(true);
    mNodeSelectorUi->bOk->setEnabled(true);

    auto type (request->getType());
    auto errorCode (e->getErrorCode());

    if (errorCode != mega::MegaError::API_OK)
    {
        mNodeSelectorUi->tMegaFolders->setEnabled(true);
        QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                                  tr("Error") + QLatin1String(": ")
                                  + QCoreApplication::translate("MegaError", e->getErrorString()));
        return;
    }

    if(type == mega::MegaRequest::TYPE_CREATE_FOLDER)
    {
        if (errorCode == mega::MegaError::API_OK)
        {
            std::shared_ptr<mega::MegaNode> node (mMegaApi->getNodeByHandle(request->getNodeHandle()));
            if (node)
            {
                QModelIndex row = mRemoteTreeModel->insertNode(node, mSelectedItemIndex);
                setSelectedFolderHandle(node->getHandle());
                QItemSelectionModel* selModel (mNodeSelectorUi->tMegaFolders->selectionModel());
                selModel->select(row,QItemSelectionModel::ClearAndSelect);
                selModel->setCurrentIndex(row, QItemSelectionModel::ClearAndSelect);
            }
        }
    }
    else if (type == mega::MegaRequest::TYPE_REMOVE || type == mega::MegaRequest::TYPE_MOVE)
    {
        if (errorCode == mega::MegaError::API_OK)
        {
            auto parent (mRemoteTreeModel->getNode(mSelectedItemIndex.parent()));
            if (parent)
            {
                mRemoteTreeModel->removeNode(mSelectedItemIndex);
                setSelectedFolderHandle(parent->getHandle());
            }
        }
    }

    mNodeSelectorUi->tMegaFolders->setEnabled(true);
}

void NodeSelector::onCustomContextMenu(const QPoint& point)
{
    QMenu customMenu;

    std::unique_ptr<mega::MegaNode> node (mMegaApi->getNodeByHandle(mSelectedFolder));
    std::unique_ptr<mega::MegaNode> parent (mMegaApi->getParentNode(node.get()));

    if (parent && node
            && QString::fromUtf8(node->getDeviceId()).isEmpty()
            && node->getHandle() != mMyBackupsRootDirHandle)
    {        
        int access = mMegaApi->getAccess(node.get());

        if (access == mega::MegaShare::ACCESS_OWNER)
        {
            customMenu.addAction(tr("Get MEGA link"), this, &NodeSelector::onGenMEGALinkClicked);
        }

        if (access >= mega::MegaShare::ACCESS_FULL)
        {
            customMenu.addAction(tr("Delete"), this, &NodeSelector::onDeleteClicked);
        }
    }

    if (!customMenu.actions().isEmpty())
    {
        customMenu.exec(mNodeSelectorUi->tMegaFolders->mapToGlobal(point));
    }
}

void NodeSelector::onDeleteClicked()
{
    std::unique_ptr<mega::MegaNode> node (mMegaApi->getNodeByHandle(mSelectedFolder));
    int access = mMegaApi->getAccess(node.get());
    if (!node || access < mega::MegaShare::ACCESS_FULL)
    {
        return;
    }

    QPointer<NodeSelector> currentDialog = this;
    if (QMegaMessageBox::question(this,
                             QString::fromUtf8("MEGAsync"),
                             tr("Are you sure that you want to delete \"%1\"?")
                                .arg(QString::fromUtf8(node->getName())),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes)
    {
        if (!currentDialog)
        {
            return;
        }

        mNodeSelectorUi->tMegaFolders->setEnabled(false);
        mNodeSelectorUi->bNewFolder->setEnabled(false);
        mNodeSelectorUi->bOk->setEnabled(false);
        const char* name = node->getName();
        if (access == mega::MegaShare::ACCESS_FULL
                || !strcmp(name, "NO_KEY")
                || !strcmp(name, "CRYPTO_ERROR")
                || !strcmp(name, "BLANK"))
        {
            mMegaApi->remove(node.get(), mDelegateListener.get());
        }
        else
        {
            auto rubbish = MegaSyncApp->getRubbishNode();
            mMegaApi->moveNode(node.get(), rubbish.get(), mDelegateListener.get());
        }
    }
}

void NodeSelector::onGenMEGALinkClicked()
{
    std::unique_ptr<mega::MegaNode> node (mMegaApi->getNodeByHandle(mSelectedFolder));
    if (node && node->getType() != mega::MegaNode::TYPE_ROOT
            && mMegaApi->getAccess(node.get()) == mega::MegaShare::ACCESS_OWNER)
    {
        mMegaApi->exportNode(node.get());
    }
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mNodeSelectorUi->retranslateUi(this);
        mNewFolderUi->retranslateUi(mNewFolderDialog);
        mNewFolderUi->errorLabel->setText(mNewFolderUi->errorLabel->text().arg(FORBIDDEN));
        nodesReady();
    }
    QDialog::changeEvent(event);
}

void NodeSelector::onSelectionChanged(QItemSelection selectedIndexes, QItemSelection)
{
    if (!selectedIndexes.isEmpty())
    {
        mSelectedItemIndex = selectedIndexes.indexes().at(0);
        auto node (mRemoteTreeModel->getNode(mSelectedItemIndex));
        if (node)
        {
            mSelectedFolder = node->getHandle();
        }
        else
        {
            mSelectedFolder = mega::INVALID_HANDLE;
        }
        // Enable or disable folder creation button
        bool allow (mSelectedItemIndex.data(Qt::UserRole).toBool());
        mNodeSelectorUi->bNewFolder->setEnabled(allow);
        mNodeSelectorUi->bOk->setEnabled(allow);
    }
    else
    {
        mSelectedItemIndex = QModelIndex();
        mSelectedFolder = mega::INVALID_HANDLE;
    }
}

void NodeSelector::on_bNewFolder_clicked()
{
    mNewFolderUi->errorLabel->hide();
    mNewFolderUi->textLabel->show();
    mNewFolderUi->lineEdit->clear();
    mNewFolderUi->lineEdit->setFocus();

    if (!mNewFolderDialog->exec())
    {
        //dialog rejected, cancel New Folder operation
        return;
    }

    QString newFolderName = mNewFolderUi->lineEdit->text().trimmed();
    std::unique_ptr<mega::MegaNode> parent (mMegaApi->getNodeByHandle(mSelectedFolder));
    if (!parent)
    {
        auto rootNode = MegaSyncApp->getRootNode();
        if (rootNode)
        {
            parent.reset(rootNode->copy());
        }
        if (!parent)
        {
            return;
        }
        mSelectedFolder = parent->getHandle();
        mSelectedItemIndex = QModelIndex();
    }

    std::unique_ptr<mega::MegaNode> node (mMegaApi->getNodeByPath(newFolderName.toUtf8().constData(),
                                                                  parent.get()));
    if (!node || node->isFile())
    {
        mNodeSelectorUi->bNewFolder->setEnabled(false);
        mNodeSelectorUi->bOk->setEnabled(false);
        mNodeSelectorUi->tMegaFolders->setEnabled(false);
        mMegaApi->createFolder(newFolderName.toUtf8().constData(),
                               parent.get(),mDelegateListener.get());
    }
    else
    {
        for (int i = 0; i < mRemoteTreeModel->rowCount(mSelectedItemIndex); i++)
        {
            QModelIndex row = mRemoteTreeModel->index(i, 0, mSelectedItemIndex);
            auto node (mRemoteTreeModel->getNode(row));

            if (node && newFolderName.compare(QString::fromUtf8(node->getName())) == 0)
            {
                QItemSelectionModel* selModel (mNodeSelectorUi->tMegaFolders->selectionModel());
                setSelectedFolderHandle(node->getHandle());
                selModel->select(row, QItemSelectionModel::ClearAndSelect);
                selModel->setCurrentIndex(row, QItemSelectionModel::ClearAndSelect);
                break;
            }
        }
    }
}

void NodeSelector::on_bOk_clicked()
{
    std::unique_ptr<mega::MegaNode> node (mMegaApi->getNodeByHandle(mSelectedFolder));
    if (!node)
    {
        reject();
        return;
    }

    int access = mMegaApi->getAccess(node.get());
    if (mSelectMode == SelectMode::UPLOAD_SELECT
            && access < mega::MegaShare::ACCESS_READWRITE)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"),
                                 tr("You need Read & Write or Full access rights "
                                    "to be able to upload to the selected folder."),
                                 QMessageBox::Ok);
        return;

    }
    else if (mSelectMode == SelectMode::SYNC_SELECT
             && access < mega::MegaShare::ACCESS_FULL)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"),
                                 tr("You need Full access right "
                                    "to be able to sync the selected folder."),
                                 QMessageBox::Ok);
        return;
    }
    else if (mSelectMode == SelectMode::STREAM_SELECT
             && node->isFolder())
    {
        QMegaMessageBox::warning(nullptr, tr("Error"),
                                 tr("Only files can be used for streaming."),
                                 QMessageBox::Ok);
        return;
    }

    if (mSelectMode == SelectMode::SYNC_SELECT)
    {
        const char* path = mMegaApi->getNodePath(node.get());
        std::unique_ptr<mega::MegaNode> check (mMegaApi->getNodeByPath(path));
        delete [] path;
        if (!check)
        {
            QMegaMessageBox::warning(nullptr, tr("Warning"),
                                     tr("Invalid folder for synchronization.\n"
                                        "Please, ensure that you don't use characters "
                                        "like '\\' '/' or ':' in your folder names."),
                                     QMessageBox::Ok);
            return;
        }
    }

    accept();
}

void NodeSelector::setupNewFolderDialog()
{
    // Initialize the NewFolder input Dialog
    mNewFolderUi->setupUi(mNewFolderDialog);
    mNewFolderDialog->setWindowFlags(mNewFolderDialog->windowFlags()
                                     & ~Qt::WindowContextHelpButtonHint);

    mNewFolderUi->errorLabel->setText(mNewFolderUi->errorLabel->text().arg(FORBIDDEN));
    // The dialog doesn't get resized on error
    mNewFolderUi->textLabel->setMinimumSize(mNewFolderUi->errorLabel->sizeHint());

    connect(mNewFolderUi->buttonBox, &QDialogButtonBox::rejected,
            mNewFolderDialog, &QDialog::reject);
    QPushButton* okButton = mNewFolderUi->buttonBox->button(QDialogButtonBox::Ok);
    //only enabled when there's input, guards against empty folder name
    okButton->setEnabled(false);
    connect(mNewFolderUi->lineEdit, &QLineEdit::textChanged, this, [this, okButton]()
    {
        bool hasText = !mNewFolderUi->lineEdit->text().trimmed().isEmpty();
        okButton->setEnabled(hasText);
    });
    mNewFolderErrorTimer.setSingleShot(true);
    connect(&mNewFolderErrorTimer, &QTimer::timeout, this, [this]()
    {
        Utilities::animateFadeout(mNewFolderUi->errorLabel);
        // after animation is finished, hide the error label and show the original text
        // 700 magic number is how long Utilities::animateFadeout takes
        QTimer::singleShot(700, this, [this]()
        {
            mNewFolderUi->errorLabel->hide();
            mNewFolderUi->textLabel->show();
        });
    });
    connect(mNewFolderUi->buttonBox, &QDialogButtonBox::accepted, this, [this]
    {
        if(mNewFolderUi->lineEdit->text().trimmed().contains(FORBIDDEN_REGEXP))
        {
            // show error label, dialog stays open
            mNewFolderUi->textLabel->hide();
            mNewFolderUi->errorLabel->show();
            Utilities::animateFadein(mNewFolderUi->errorLabel);
            mNewFolderErrorTimer.start(NEW_FOLDER_DISPLAY_TIME_MS); //(re)start timer
            mNewFolderUi->lineEdit->setFocus();
        }
        else
        {
            //dialog accepted, execute New Folder operation
            mNewFolderDialog->accept();
        }
    });
}

bool NodeSelector::getDefaultUploadOption()
{
   return mNodeSelectorUi->cbAlwaysUploadToLocation->isChecked();
}

void NodeSelector::onMyBackupsRootDir(mega::MegaHandle handle)
{
    mMyBackupsRootDirHandle = handle;
}
