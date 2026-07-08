#include "FilePickerNodeSelectorSpecializations.h"

#include "megaapi.h"
#include "MessageDialogOpener.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "SyncInfo.h"
#include "ui_NodeSelector.h"

#include <QPointer>

UploadNodeSelector::UploadNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new UploadType), parent)
{}

QString UploadNodeSelector::destinationBreadcrumbEmptyText()
{
    return tr("Select a shared folder to upload your items to");
}

void UploadNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if (node)
    {
        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_READWRITE)
        {
            MessageDialogInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.descriptionText =
                tr("You need Read & Write or Full access rights to be able to "
                   "upload to the selected folder.");
            msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
            {
                reject();
            };
            MessageDialogOpener::warning(msgInfo);
        }
        else
        {
            accept();
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

DownloadNodeSelector::DownloadNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new DownloadType), parent)
{
    setWindowTitle(tr("Download"));
}

void DownloadNodeSelector::onOkButtonClicked()
{
    QList<mega::MegaHandle> nodes = getMultiSelectionNodeHandle();
    int wrongNodes(0);
    foreach(auto& nodeHandle, nodes)
    {
        auto node = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(nodeHandle));
        if (!node)
        {
            ++wrongNodes;
        }
    }

    MessageDialogInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
    {
        reject();
    };

    if (wrongNodes == 0)
    {
        accept();
    }
    else
    {
        if (wrongNodes == nodes.size())
        {
            if (ui->stackedWidget->currentIndex() ==
                NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE)
            {
                msgInfo.descriptionText =
                    tr("The item you selected has been removed. To reselect, close "
                       "this window and try again.",
                       "",
                       wrongNodes);
                MessageDialogOpener::warning(msgInfo);
            }
            else
            {
                msgInfo.descriptionText =
                    tr("You no longer have access to this item. Ask the owner to share again.",
                       "",
                       wrongNodes);
                MessageDialogOpener::warning(msgInfo);
            }
        }
        else
        {
            QString warningMsg1 = tr("%1 item selected", "", static_cast<int>(nodes.size()))
                                      .arg(static_cast<int>(nodes.size()));
            msgInfo.descriptionText =
                tr("%1. %2 has been removed. To reselect, close this window and try again.",
                   "",
                   wrongNodes)
                    .arg(warningMsg1)
                    .arg(wrongNodes);
            MessageDialogOpener::warning(msgInfo);
        }
    }
}

SyncNodeSelector::SyncNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new SyncType), parent)
{
    if (mIncomingSharesWidget)
    {
        connect(mIncomingSharesWidget,
                &NodeSelectorTreeViewWidget::viewStateChanged,
                this,
                &SyncNodeSelector::refreshDestinationBreadcrumb);
    }
}

QString SyncNodeSelector::destinationBreadcrumbEmptyText()
{
    return tr("Select a full access shared folder to sync");
}

void SyncNodeSelector::onModelModified()
{
    // Syncs is the only FilePicker that can show/hide the breadcrumb/banner depending on the model
    refreshDestinationBreadcrumb();
}

void SyncNodeSelector::refreshDestinationBreadcrumb()
{
    // Case 3: SHARES tab with no shares -> hide breadcrumb and banner; the tree view
    // shows its own empty state.
    if (incomingSharesTabIsEmpty())
    {
        ui->destinationBanner->setVisible(false);
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setSegments({});
        return;
    }

    FilePickerNodeSelector::refreshDestinationBreadcrumb();
}

bool SyncNodeSelector::incomingSharesTabIsEmpty() const
{
    auto* currentWidget = getCurrentTreeViewWidget();
    if (!currentWidget ||
        currentWidget->getTabType() != NodeSelectorTreeViewWidget::TabItem::SHARES)
    {
        return false;
    }

    auto* proxy = currentWidget->getProxyModel();
    if (!proxy)
    {
        return false;
    }

    return proxy->rowCount(proxy->getTopRootIndex()) == 0;
}

bool SyncNodeSelector::isFullSync()
{
    auto syncsList = SyncInfo::instance()->getSyncSettingsByType(SyncInfo::SyncType::TYPE_TWOWAY);
    auto foundIt =
        std::find_if(syncsList.cbegin(),
                     syncsList.cend(),
                     [](const auto& sync)
                     {
                         return (sync->getMegaFolder() == QLatin1String("/") && sync->isActive());
                     });

    return foundIt != syncsList.cend();
}

QString SyncNodeSelector::destinationTitleText() const
{
    return tr("Folder to sync");
}

FilePickerNodeSelector::DestinationBannerInfo SyncNodeSelector::destinationBannerInfo() const
{
    DestinationBannerInfo info{BannerWidget::Type::BANNER_WARNING, {}};

    auto* contextWidget = selectedSearchChipTreeViewWidget();
    if (!contextWidget)
    {
        return info;
    }

    // While searching, the selection lives in the search widget, not in the chip returned
    // by selectedSearchChipTreeViewWidget() (the selected chip); consult the currently
    // visible widget.
    auto* selectionWidget = getCurrentTreeViewWidget();
    const auto selectedIndexes =
        selectionWidget ? selectionWidget->getSelectedIndexes() : QModelIndexList();
    if (selectedIndexes.size() == 1)
    {
        const auto status = selectedIndexes.first()
                                .data(toInt(NodeSelectorModelRoles::STATUS_ROLE))
                                .value<NodeSelectorModelItem::Status>();

        switch (status)
        {
            case NodeSelectorModelItem::Status::SYNC:
            case NodeSelectorModelItem::Status::SYNC_CHILD:
            {
                info.text = tr("Choose a different folder. This folder is already synced");
                break;
            }
            case NodeSelectorModelItem::Status::SYNC_PARENT:
            {
                info.text = tr("Choose a different folder. This location contains a folder that's "
                               "already synced");
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else if (contextWidget->getTabType() == NodeSelectorTreeViewWidget::TabItem::SHARES)
    {
        if (!fullAccessInTopRootShares())
        {
            info.text = tr("Only shared folders with full access can be synced");
        }
        else if (!enableFoldersInTopRootShares())
        {
            info.text = tr("Choose a different folder. This location contains a folder that's "
                           "already synced");
        }
    }

    return info;
}

bool SyncNodeSelector::fullAccessInTopRootShares() const
{
    if (!mIncomingSharesWidget)
    {
        return false;
    }

    auto* proxy = mIncomingSharesWidget->getProxyModel();
    if (!proxy)
    {
        return false;
    }

    const auto topRoot = proxy->getTopRootIndex();
    const int rowCount = proxy->rowCount(topRoot);
    if (rowCount == 0)
    {
        return false;
    }

    for (int row = 0; row < rowCount; ++row)
    {
        const auto idx = proxy->index(row, 0, topRoot);
        if (idx.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt() ==
            mega::MegaShare::ACCESS_FULL)
        {
            return true;
        }
    }

    return false;
}

bool SyncNodeSelector::enableFoldersInTopRootShares() const
{
    if (!mIncomingSharesWidget)
    {
        return false;
    }

    auto* proxy = mIncomingSharesWidget->getProxyModel();
    if (!proxy)
    {
        return false;
    }

    const auto topRoot = proxy->getTopRootIndex();
    const int rowCount = proxy->rowCount(topRoot);
    if (rowCount == 0)
    {
        return false;
    }

    for (int row = 0; row < rowCount; ++row)
    {
        const auto idx = proxy->index(row, 0, topRoot);
        if (idx.flags() & Qt::ItemIsEnabled)
        {
            return true;
        }
    }

    return false;
}

void SyncNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if (node)
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
        {
            reject();
        };

        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_FULL)
        {
            msgInfo.descriptionText =
                tr("You need Full access right to be able to sync the selected folder.");
            MessageDialogOpener::warning(msgInfo);
        }
        else
        {
            std::unique_ptr<char[]> path(mMegaApi->getNodePath(node.get()));
            auto check = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.get()));
            if (!check)
            {
                msgInfo.descriptionText =
                    tr("Invalid folder for synchronization.\n"
                       "Please, ensure that you don't use characters like '\\' '/' "
                       "or ':' in your folder names.");
                MessageDialogOpener::warning(msgInfo);
            }
            else
            {
                accept();
            }
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

StreamNodeSelector::StreamNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new StreamType), parent)
{}

QString StreamNodeSelector::destinationTitleText() const
{
    return tr("Select a file to stream");
}

FilePickerNodeSelector::DestinationBannerInfo StreamNodeSelector::destinationBannerInfo() const
{
    auto selectedNode(getSelectedNode());

    if (selectedNode && selectedNode->isFile())
    {
        return {};
    }

    return {BannerWidget::Type::BANNER_MESSAGE, tr("Select a file to stream")};
}

void StreamNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if (node)
    {
        if (node->isFolder())
        {
            MessageDialogInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.descriptionText = tr("Only files can be used for streaming.");
            msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
            {
                reject();
            };
            MessageDialogOpener::warning(msgInfo);
        }
        else
        {
            accept();
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

MoveBackupNodeSelector::MoveBackupNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new MoveBackupType), parent)
{}

void MoveBackupNodeSelector::onOkButtonClicked()
{
    accept();
}
