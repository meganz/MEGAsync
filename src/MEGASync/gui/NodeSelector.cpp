#include "NodeSelector.h"
#include "ui_NodeSelector.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"
#include "MegaItemProxyModel.h"
#include "MegaItemModel.h"
#include "megaapi.h"
#include "MegaItemDelegates.h"
#include "mega/utils.h"

#include <QMessageBox>
#include <QPointer>
#include <QShortcut>

using namespace mega;

const int NodeSelector::LABEL_ELIDE_MARGIN = 100;

const char* NodeSelector::IN_SHARES = "Incoming shares";
const char* NodeSelector::CLD_DRIVE = "Cloud drive";


NodeSelector::NodeSelector(int selectMode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeSelector),
    mSelectMode(selectMode),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowModality(Qt::WindowModal);
    ui->setupUi(this);

    ui->cbAlwaysUploadToLocation->hide();
    ui->bOk->setDefault(true);

#ifndef Q_OS_MAC
    ui->bShowCloudDrive->setChecked(true);
    connect(ui->bShowIncomingShares, &QPushButton::clicked, this, &NodeSelector::onbShowIncomingSharesClicked);
    connect(ui->bShowCloudDrive, &QPushButton::clicked,this , &NodeSelector::onbShowCloudDriveClicked);
#else
    ui->tabBar->addTab(tr(CLD_DRIVE));
    ui->tabBar->addTab(tr(IN_SHARES));
    connect(ui->tabBar, &QTabBar::currentChanged, this, &NodeSelector::onTabSelected);
#endif

    nodesReady();
    ui->bOk->setEnabled(false);
    connect(ui->bNewFolder, &QPushButton::clicked, this, &NodeSelector::onbNewFolderClicked);
    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelector::onbOkClicked);
    connect(ui->bCancel, &QPushButton::clicked, this, &QDialog::reject);

    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i < 2; ++i)
    {
        QShortcut *shortcut = new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i+1)), this);
        QObject::connect(shortcut, &QShortcut::activated, this, [=](){ onTabSelected(i); });
    }
}

NodeSelector::~NodeSelector()
{
    delete ui;
}

void NodeSelector::nodesReady()
{
    if (!mMegaApi->isFilesystemAvailable())
    {
        ui->bOk->setEnabled(false);
        ui->bNewFolder->setEnabled(false);
        return;
    }

    ui->stackedWidget->setProperty("SelectionMode", mSelectMode);
    switch(mSelectMode)
    {
    case NodeSelector::SYNC_SELECT:
        // fall through
    case NodeSelector::UPLOAD_SELECT:
        ui->bNewFolder->show();
        break;
    case NodeSelector::DOWNLOAD_SELECT:
        ui->bNewFolder->hide();
        break;
    case NodeSelector::STREAM_SELECT:
        ui->bNewFolder->hide();
        setWindowTitle(tr("Select items"));
        break;
    }

//Disable animation for OS X due to problems showing the tree icons
#ifdef __APPLE__
    ui->tMegaFolders->setAnimated(false);
#endif
}

void NodeSelector::onbNewFolderClicked()
{
    auto tree_view_widget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
    tree_view_widget->newFolderClicked();
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    ui->cbAlwaysUploadToLocation->setVisible(show);
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void NodeSelector::onbOkClicked()
{
    bool correctNodeSelected(true);

    if(mSelectMode == NodeSelector::DOWNLOAD_SELECT)
    {
        auto treeViewWidget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
        auto nodes = treeViewWidget->getMultiSelectionNodeHandle();
        int wrongNodes(0);
        foreach(auto& nodeHandle, nodes)
        {
            auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(nodeHandle));
            if(!node)
            {
                ++wrongNodes;
            }
        }

        if(wrongNodes == nodes.size())
        {
            correctNodeSelected = false;
            //TODO EKA: fix this message as isclouddrive is not available anymore
//            if(isCloudDrive())
//            {
//                QMegaMessageBox::warning(nullptr, tr("Error"), tr("The item you selected has been removed. To reselect, close this window and try again.", "", wrongNodes), QMessageBox::Ok);
//            }
//            else
//            {
//                QMegaMessageBox::warning(nullptr, tr("Error"), tr("You no longer have access to this item. Ask the owner to share again.", "", wrongNodes), QMessageBox::Ok);
//            }
        }
        else if(wrongNodes > 0)
        {
            correctNodeSelected = false;
            QString warningMsg1 = tr("%1 item selected", "", nodes.size()).arg(nodes.size());
            QString warningMsg = tr("%1. %2 has been removed. To reselect, close this window and try again.", "", wrongNodes).arg(warningMsg1).arg(wrongNodes);
            QMegaMessageBox::warning(nullptr, tr("Error"), warningMsg, QMessageBox::Ok);
        }
    }
    else
    {
        auto treeViewWidget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
        auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(treeViewWidget->getSelectedNodeHandle()));
        if (!node)
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("The item you selected has been removed. To reselect, close this window and try again."),
                                                 QMessageBox::Ok);
            correctNodeSelected = false;
        }
        else
        {
            int access = mMegaApi->getAccess(node.get());
            if ((mSelectMode == NodeSelector::UPLOAD_SELECT) && ((access < MegaShare::ACCESS_READWRITE)))
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Read & Write or Full access rights to be able to upload to the selected folder."), QMessageBox::Ok);
                correctNodeSelected = false;
            }
            else if ((mSelectMode == NodeSelector::SYNC_SELECT) && (access < MegaShare::ACCESS_FULL))
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Full access right to be able to sync the selected folder."), QMessageBox::Ok);
                correctNodeSelected = false;
            }
            else if ((mSelectMode == NodeSelector::STREAM_SELECT) && node->isFolder())
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("Only files can be used for streaming."), QMessageBox::Ok);
                correctNodeSelected = false;
            }
            else if (mSelectMode == NodeSelector::SYNC_SELECT)
            {
                const char* path = mMegaApi->getNodePath(node.get());
                auto check = std::unique_ptr<MegaNode>(mMegaApi->getNodeByPath(path));
                delete [] path;
                if (!check)
                {
                    QMegaMessageBox::warning(nullptr, tr("Warning"), tr("Invalid folder for synchronization.\n"
                                                         "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names."),
                                             QMessageBox::Ok);
                    correctNodeSelected = false;
                }
            }
        }
    }

    correctNodeSelected ? accept() : reject();
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    ui->stackedWidget->setCurrentIndex(SHARES);
//    if(mProxyModel)
//    {
//        saveExpandedItems();
//        mProxyModel->showOnlyInShares(mSelectMode == NodeSelector::SYNC_SELECT);
//        restoreExpandedItems();
//        checkNewFolderButtonVisibility();
//        checkBackForwardButtons();
//    }
}

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
//    if(mProxyModel)
//    {
//        saveExpandedItems();
//        mProxyModel->showOnlyCloudDrive();
//        restoreExpandedItems();
//        checkNewFolderButtonVisibility();
//        checkBackForwardButtons();
//    }
}

void NodeSelector::onTabSelected(int index)
{
    switch (index)
    {
        case NodeSelector::CLOUD_DRIVE:
#ifdef Q_OS_MAC
            onbShowCloudDriveClicked();
            ui->tabBar->setCurrentIndex(index);
#else
            ui->bShowCloudDrive->click();
#endif
            break;
        case NodeSelector::SHARES:
#ifdef Q_OS_MAC
            onbShowIncomingSharesClicked();
            ui->tabBar->setCurrentIndex(index);
#else
            ui->bShowIncomingShares->click();
#endif
            break;
        default:
            break;
    }
}

//REQUIRED WITH NEW ARCHITECTURE?
//void NodeSelector::saveExpandedItems()
//{
//    auto node = mProxyModel->getNode(ui->tMegaFolders->rootIndex());

//    if(isCloudDrive())
//    {
//        mNavCloudDrive.rootHandle = node? node->getHandle() : INVALID_HANDLE;
//        iterateForSaveExpanded(mNavCloudDrive.expandedHandles);
//    }
//    else
//    {
//        mNavInShares.rootHandle  = node? node->getHandle() : INVALID_HANDLE;
//        iterateForSaveExpanded(mNavInShares.expandedHandles);
//    }
//}

//REQUIRED WITH NEW ARCHITECTURE?
//void NodeSelector::iterateForSaveExpanded(QList<MegaHandle> &saveList, const QModelIndex& parent)
//{
//    for(int i=0; i < mProxyModel->rowCount(parent); ++i)
//    {
//        auto idx = mProxyModel->index(i, 0, parent);
//        if(idx.isValid() && ui->tMegaFolders->isExpanded(idx))
//        {
//            saveList.append(mProxyModel->getNode(idx)->getHandle());
//            iterateForSaveExpanded(saveList, idx);
//        }
//    }
//}

//void NodeSelector::restoreExpandedItems()
//{
//    if(isCloudDrive())
//    {
//        auto idx = mProxyModel->getIndexFromHandle(mNavCloudDrive.rootHandle);
//        setRootIndex(idx);
//        iterateForRestore(mNavCloudDrive.expandedHandles);
//        mNavCloudDrive.expandedHandles.clear();
//    }
//    else
//    {
//        auto idx = mProxyModel->getIndexFromHandle(mNavInShares.rootHandle);
//        setRootIndex(idx);
//        iterateForRestore(mNavInShares.expandedHandles);
//        mNavInShares.expandedHandles.clear();
//    }
//}

//void NodeSelector::iterateForRestore(const QList<MegaHandle> &list, const QModelIndex &parent)
//{
//    if(list.isEmpty())
//        return;

//    for(int i=0; i < mProxyModel->rowCount(parent); ++i)
//    {
//        auto idx = mProxyModel->index(i, 0, parent);
//        if(idx.isValid() && list.contains(mProxyModel->getNode(idx)->getHandle()))
//        {
//           ui->tMegaFolders->expand(idx);
//           iterateForRestore(list, idx);
//        }
//    }
//}

bool NodeSelector::getDefaultUploadOption()
{
    return ui->cbAlwaysUploadToLocation->isChecked();
}

MegaHandle NodeSelector::getSelectedNodeHandle()
{
    auto tree_view = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
    return tree_view->getSelectedNodeHandle();
}

QList<MegaHandle> NodeSelector::getMultiSelectionNodeHandle()
{
    auto tree_view = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
    return tree_view->getMultiSelectionNodeHandle();
}

void NodeSelector::setSelectedNodeHandle(const mega::MegaHandle &handle)
{
    auto parent_node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(handle));
    while(parent_node && parent_node->getParentHandle() != INVALID_HANDLE)
    {
        parent_node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(parent_node->getParentHandle()));
    }

    if(!parent_node)
    {
        return;
    }
    //TODO EKA:
    if(parent_node->isInShare())
    {
        //openInshare
    }
    else
    {
        //open cloud drive
    }
    auto tree_view_widget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(0));
    tree_view_widget->setSelectedNodeHandle(handle);
}
