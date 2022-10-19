#include "NodeSelector.h"
#include "ui_NodeSelector.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"
#include "megaapi.h"
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

    ui->CloudDrive->setSelectionMode(mSelectMode);
    ui->IncomingShares->setSelectionMode(mSelectMode);

    connect(ui->CloudDrive, &NodeSelectorTreeViewWidget::okBtnClicked, this, &NodeSelector::onbOkClicked);
    connect(ui->IncomingShares, &NodeSelectorTreeViewWidget::okBtnClicked, this, &NodeSelector::onbOkClicked);
    connect(ui->CloudDrive, &NodeSelectorTreeViewWidget::cancelBtnClicked, this, &NodeSelector::reject);
    connect(ui->IncomingShares, &NodeSelectorTreeViewWidget::cancelBtnClicked, this, &NodeSelector::reject);

#ifndef Q_OS_MAC
    ui->bShowCloudDrive->setChecked(true);
    connect(ui->bShowIncomingShares, &QPushButton::clicked, this, &NodeSelector::onbShowIncomingSharesClicked);
    connect(ui->bShowCloudDrive, &QPushButton::clicked,this , &NodeSelector::onbShowCloudDriveClicked);
#else
    ui->tabBar->addTab(tr(CLD_DRIVE));
    ui->tabBar->addTab(tr(IN_SHARES));
    connect(ui->tabBar, &QTabBar::currentChanged, this, &NodeSelector::onTabSelected);
#endif

    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i < 2; ++i)
    {
        QShortcut *shortcut = new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i+1)), this);
        QObject::connect(shortcut, &QShortcut::activated, this, [=](){ onTabSelected(i); });
    }
    onbShowCloudDriveClicked();

    //TODO EKA: WE need to do this at this lvl? only for stream_select mode, switch removed
    //setWindowTitle(tr("Select items"));
    if(mSelectMode == STREAM_SELECT)
    {
        setWindowTitle(tr("Select items"));
    }

}

NodeSelector::~NodeSelector()
{
    delete ui;
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    ui->CloudDrive->showDefaultUploadOption(show);
    ui->IncomingShares->showDefaultUploadOption(show);
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    ui->CloudDrive->setDefaultUploadOption(value);
    ui->IncomingShares->setDefaultUploadOption(value);
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void NodeSelector::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
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
            if(ui->stackedWidget->currentIndex() == CLOUD_DRIVE)
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("The item you selected has been removed. To reselect, close this window and try again.", "", wrongNodes), QMessageBox::Ok);
            }
            else
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("You no longer have access to this item. Ask the owner to share again.", "", wrongNodes), QMessageBox::Ok);
            }
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
}

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
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

void NodeSelector::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)
    if(mSelectMode == UPLOAD_SELECT || mSelectMode == DOWNLOAD_SELECT)
    {
        ui->bOk->setEnabled(!selected.indexes().isEmpty());
        return;
    }

    ui->bOk->setEnabled(false);
    foreach(auto& index, selected.indexes())
    {
        if(index.column() != MegaItemModel::COLUMN::NODE)
        {
            continue;
        }

        auto source_idx = mProxyModel->getIndexFromSource(index);
        MegaItem *item = static_cast<MegaItem*>(source_idx.internalPointer());
        if(item)
        {
            if(mSelectMode == NodeSelector::STREAM_SELECT)
            {
                ui->bOk->setEnabled(item->getNode()->isFile());
            }
            else if(mSelectMode == NodeSelector::SYNC_SELECT)
            {
                ui->bOk->setEnabled(item->isSyncable());
            }
        }
    }
}

void NodeSelector::onSectionResized()
{
    if(!mManuallyResizedColumn
            && ui->tMegaFolders->header()->rect().contains(ui->tMegaFolders->mapFromGlobal(QCursor::pos())))
    {
        mManuallyResizedColumn = true;
    }
}

void NodeSelector::saveExpandedItems()
{
    auto node = mProxyModel->getNode(ui->tMegaFolders->rootIndex());

    if(isCloudDrive())
    {
        mNavCloudDrive.rootHandle = node? node->getHandle() : INVALID_HANDLE;
        iterateForSaveExpanded(mNavCloudDrive.expandedHandles);
    }
    else
    {
        mNavInShares.rootHandle  = node? node->getHandle() : INVALID_HANDLE;
        iterateForSaveExpanded(mNavInShares.expandedHandles);
    }
}

void NodeSelector::iterateForSaveExpanded(QList<MegaHandle> &saveList, const QModelIndex& parent)
{
    for(int i=0; i < mProxyModel->rowCount(parent); ++i)
    {
        auto idx = mProxyModel->index(i, 0, parent);
        if(idx.isValid() && ui->tMegaFolders->isExpanded(idx))
        {
            saveList.append(mProxyModel->getNode(idx)->getHandle());
            iterateForSaveExpanded(saveList, idx);
        }
    }
}

void NodeSelector::restoreExpandedItems()
{
    if(isCloudDrive())
    {
        auto idx = mProxyModel->getIndexFromHandle(mNavCloudDrive.rootHandle);
        setRootIndex(idx);
        iterateForRestore(mNavCloudDrive.expandedHandles);
        mNavCloudDrive.expandedHandles.clear();
    }
    else
    {
        auto idx = mProxyModel->getIndexFromHandle(mNavInShares.rootHandle);
        setRootIndex(idx);
        iterateForRestore(mNavInShares.expandedHandles);
        mNavInShares.expandedHandles.clear();
    }
}

void NodeSelector::iterateForRestore(const QList<MegaHandle> &list, const QModelIndex &parent)
{
    //TODO FIX THIS as checkbox has been moved to stack page
    //return ui->cbAlwaysUploadToLocation->isChecked();
    return false;
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
    if(parent_node->isInShare())
    {
        onTabSelected(SHARES);
    }
    else
    {
        onTabSelected(CLOUD_DRIVE);
    }
    auto tree_view_widget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
    tree_view_widget->setFutureSelectedNodeHandle(handle);
    //tree_view_widget->setSelectedNodeHandle(handle);
}
