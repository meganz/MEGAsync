#include "FilePickerNodeSelector.h"

#include "DestinationBreadcrumb.h"
#include "megaapi.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelector.h"

#include <QCoreApplication>

FilePickerNodeSelector::FilePickerNodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    NodeSelector(selectType, parent)
{
    connect(ui->destinationBreadcrumb,
            &DestinationBreadcrumb::clearRequested,
            this,
            &FilePickerNodeSelector::onBreadcrumbClearRequested);
    connect(ui->destinationBreadcrumb,
            &DestinationBreadcrumb::refreshNeeded,
            this,
            &FilePickerNodeSelector::refreshDestinationBreadcrumb);
}

void FilePickerNodeSelector::onNodesRenamed(const QList<mega::MegaHandle>& handles)
{
    ui->destinationBreadcrumb->onNodesRenamed(handles);
}

std::shared_ptr<mega::MegaNode>
    FilePickerNodeSelector::getNewFolderParentNode(NodeSelectorTreeViewWidget* sourceWidget) const
{
    // File pickers have no folder navigation: create the folder inside the selected folder, or
    // fall back to the top root when nothing (or a file) is selected.
    auto selectedHandle = sourceWidget->getSelectedNodeHandle();
    if (selectedHandle != mega::INVALID_HANDLE)
    {
        std::shared_ptr<mega::MegaNode> selectedNode(mMegaApi->getNodeByHandle(selectedHandle));
        if (selectedNode && selectedNode->isFolder())
        {
            return selectedNode;
        }
    }

    return NodeSelector::getNewFolderParentNode(sourceWidget);
}

void FilePickerNodeSelector::applyNewFolderSelection(NodeSelectorTreeViewWidget* sourceWidget,
                                                     mega::MegaNode* newNode)
{
    // The parent is initialised before the node arrives (expandNodeByHandle on creatingFolder),
    // so the new folder comes through the visible add path; mark it for checkNewFolderAdded to
    // select once nodesAdded arrives.
    sourceWidget->setNewFolderInfo({newNode->getHandle(), true});
}

void FilePickerNodeSelector::refreshDestinationBreadcrumb()
{
    // Building the destination path goes through the SDK (getNodeByHandle/getNodePath). While
    // the model's worker thread is fetching children, getChildren holds the SDK mutex for
    // hundreds of ms on huge folders, so those calls would freeze the GUI thread and prevent
    // the loading view from showing. Skip the refresh in that case: the post-load selection
    // pass (onUiBlocked(false) -> onSelectionHasChanged) runs it again once the fetch is done.
    if (auto* currentWidget = getCurrentTreeViewWidget())
    {
        const auto proxyModel = currentWidget->getProxyModel();
        if (proxyModel && proxyModel->getMegaModel() &&
            proxyModel->getMegaModel()->isRequestingNodes())
        {
            return;
        }
    }

    const bool shouldShowPath = mSelectType && mSelectType->showsDestinationBreadcrumb() &&
                                selectedSearchChipTreeViewWidget();
    const auto banner = destinationBannerInfo();

    const bool shouldShowBanner = !banner.text.isEmpty() && banner.type != BannerWidget::Type::NONE;

    // Mutually exclusive: banner wins over breadcrumb when both would apply.
    if (shouldShowBanner)
    {
        ui->destinationBanner->setType(banner.type);
        ui->destinationBanner->setTitle(banner.text);
        ui->destinationBanner->setVisible(true);
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setSegments({});
        return;
    }

    ui->destinationBanner->setVisible(false);

    if (!shouldShowPath)
    {
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setSegments({});
        return;
    }

    ui->destinationBreadcrumb->setVisible(true);
    auto path{destinationBreadcrumbSegments()};
    if (path.isEmpty())
    {
        ui->destinationBreadcrumb->setSegments(
            {{mega::INVALID_HANDLE, destinationBreadcrumbEmptyText()}});
    }
    else
    {
        ui->destinationBreadcrumb->setSegments(path);
    }
    ui->destinationBreadcrumb->setTitleText(destinationTitleText());
}

bool FilePickerNodeSelector::shouldClearSelectionOnBackgroundClick(const QPoint& pos) const
{
    const bool keepSelectionOnRightPaneClick = mSelectType &&
                                               mSelectType->showsDestinationBreadcrumb() &&
                                               ui->wRightPaneNS->geometry().contains(pos);

    return !keepSelectionOnRightPaneClick;
}

void FilePickerNodeSelector::onLanguageChangeEvent()
{
    refreshDestinationBreadcrumb();
}

void FilePickerNodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::INCREASE, true, true);
}

void FilePickerNodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles,
                                                         int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::DECREASE, true, true);
}

void FilePickerNodeSelector::onBreadcrumbClearRequested()
{
    // If there is no selection, move to top root indexpo
    if (auto* wid = getCurrentTreeViewWidget())
    {
        if (!wid->clearSelection())
        {
            wid->moveToTopRootIndex();
        }
    }
}

void FilePickerNodeSelector::configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE, true);
    widget->setColumnHidden(NodeSelectorModel::Column::LAST_MODIFIED_DATE, true);
}

void FilePickerNodeSelector::configureSidebar()
{
    static constexpr int COLLAPSED_SIDEBAR_WIDTH = 60;
    static constexpr int COLLAPSED_TAB_SIZE = 36;
    static constexpr int COLLAPSED_ICON_SIZE = 20;

    ui->wLeftPaneNS->setMinimumWidth(COLLAPSED_SIDEBAR_WIDTH);
    ui->wLeftPaneNS->setMaximumWidth(COLLAPSED_SIDEBAR_WIDTH);

    const auto collapseTab = [](TabSelector* tab)
    {
        if (!tab)
        {
            return;
        }
        tab->setIconOnly(true);
        tab->setProperty("class", QLatin1String("sidebar-icononly"));
        tab->setFixedSize(COLLAPSED_TAB_SIZE, COLLAPSED_TAB_SIZE);
        tab->setIconSize(QSize(COLLAPSED_ICON_SIZE, COLLAPSED_ICON_SIZE));
        tab->style()->unpolish(tab);
        tab->style()->polish(tab);
    };

    collapseTab(ui->fCloudDrive);
    collapseTab(ui->fIncomingShares);
    collapseTab(ui->fBackups);
    collapseTab(ui->fRubbish);

    const auto applyHeaderStyle = [](TokenizableButton* btn)
    {
        if (!btn)
        {
            return;
        }
        btn->setIcon(QIcon());
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    };

    for (auto* btn: {ui->bUpload, ui->bNewFolder, ui->bClearRubbish})
    {
        applyHeaderStyle(btn);
    }

    // The account info widget is only shown in the file manager (Cloud Drive).
    ui->wAccountInfo->setVisible(false);
}

void FilePickerNodeSelector::configureHeader()
{
    ui->leSearchTool->setMode(SearchLineEdit::Mode::ALWAYS_EXPANDED);

    static constexpr int SEARCH_LINE_EDIT_FIXED_WIDTH = 188;
    static constexpr int SEARCH_LINE_EDIT_FIXED_HEIGHT = 40;

    ui->leSearchTool->setFixedSize(SEARCH_LINE_EDIT_FIXED_WIDTH, SEARCH_LINE_EDIT_FIXED_HEIGHT);
    ui->actionButtonsContainer->setVisible(false);
    ui->navigationBreadcrumb->setVisible(false);
    ui->lNavigationRoot->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    if (auto* topRowLayout = ui->headerLayout)
    {
        ui->navigationRowLayout->removeWidget(ui->lNavigationRoot);
        // The 8px indent only compensates the navigation row's reduced left margin; the
        // header top row keeps the full 20px margin.
        ui->lNavigationRoot->setIndent(0);
        // Vertically centered so the label sits on the same line as the 40px search field.
        topRowLayout->insertWidget(0, ui->lNavigationRoot, 0, Qt::AlignLeft | Qt::AlignVCenter);
    }
}

void FilePickerNodeSelector::refreshNavigationBreadcrumb()
{
    auto* widget = getCurrentTreeViewWidget();
    const bool show = widget && widget != mSearchWidget;

    ui->navigationBreadcrumb->setVisible(false);
    ui->lNavigationRoot->setVisible(show);
    if (show)
    {
        ui->lNavigationRoot->setText(widget->getRootText());
    }
}

QString FilePickerNodeSelector::destinationTitleText() const
{
    return QCoreApplication::translate("DestinationBreadcrumb", "Destination");
}

FilePickerNodeSelector::DestinationBannerInfo FilePickerNodeSelector::destinationBannerInfo() const
{
    return {};
}

QList<NodeSelectorBreadcrumbSegment> FilePickerNodeSelector::destinationBreadcrumbSegments() const
{
    auto* destinationWidget = selectedSearchChipTreeViewWidget();
    if (!destinationWidget)
    {
        return {};
    }

    // Selection lives in the currently visible widget (the search widget when
    // searching); destinationWidget (the selected chip) only provides the root
    // context for the path.
    auto* selectionWidget = getCurrentTreeViewWidget();
    if (!selectionWidget)
    {
        return {};
    }

    auto nodeHandle = selectionWidget->getSelectedNodeHandle();
    if (nodeHandle != mega::INVALID_HANDLE)
    {
        MegaNodeSPtr node(mMegaApi->getNodeByHandle(nodeHandle));
        if (node)
        {
            return breadcrumbSegmentsForNode(destinationWidget, node);
        }
    }

    return {};
}

QList<NodeSelectorBreadcrumbSegment> FilePickerNodeSelector::breadcrumbSegmentsForNode(
    NodeSelectorTreeViewWidget* wid,
    const std::shared_ptr<mega::MegaNode>& node) const
{
    if (!wid)
    {
        return {};
    }

    // Root label is a synthetic tab name, not a node: no handle, only fallback text.
    QList<NodeSelectorBreadcrumbSegment> segments;
    segments.append({mega::INVALID_HANDLE, wid->getRootText()});
    if (!node)
    {
        return segments;
    }

    std::unique_ptr<char[]> path(mMegaApi->getNodePath(node.get()));
    if (!path)
    {
        return segments;
    }

    const auto names = QString::fromUtf8(path.get()).split(QLatin1Char('/'), Qt::SkipEmptyParts);

    // Walk up from the node pairing each path component with its handle, so the breadcrumb
    // can resolve fresh names later (e.g. when the overflow popup opens). names is top-down
    // while the walk is bottom-up, so iterate names from the deepest end to keep each name
    // aligned with its own handle. If an ancestor cannot be retrieved the walk stops and the
    // remaining (top) components fall back to INVALID_HANDLE.
    QList<NodeSelectorBreadcrumbSegment> nodeSegments;
    std::shared_ptr<mega::MegaNode> current = node;
    for (int i = names.size() - 1; i >= 0; --i)
    {
        const auto handle = current ? current->getHandle() : mega::INVALID_HANDLE;
        nodeSegments.prepend({handle, names.at(i)});

        if (current)
        {
            current.reset(mMegaApi->getNodeByHandle(current->getParentHandle()));
        }
    }

    segments.append(nodeSegments);

    return segments;
}

std::shared_ptr<mega::MegaNode>
    FilePickerNodeSelector::destinationNodeForBreadcrumb(NodeSelectorTreeViewWidget* wid) const
{
    if (!wid)
    {
        return nullptr;
    }

    if (auto selectedNode = getSelectedNode())
    {
        return selectedNode;
    }

    if (!mSelectType)
    {
        return nullptr;
    }

    const auto rootIndex = wid->getCurrentRootIndex();
    if (rootIndex.isValid())
    {
        return wid->getProxyModel()->getNode(rootIndex);
    }

    return nullptr;
}
