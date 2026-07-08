#include "NodeSelectorSearchController.h"

#include "megaapi.h"
#include "NodeSelectorModelSpecialised.h"
#include "NodeSelectorProxyModel.h"
#include "TabSelector.h"
#include "ui_NodeSelectorTreeViewWidget.h"

namespace
{
constexpr const char* TAB_TYPE_PROPERTY = "search_tab_type";
}

NodeSelectorSearchController::NodeSelectorSearchController(Ui::NodeSelectorTreeViewWidget* ui):
    mUi(ui)
{
    initialize();
}

void NodeSelectorSearchController::initialize()
{
    setTabTypeProperty(TabType::CLOUD_DRIVE, mUi->cloudDriveSearch);
    setTabTypeProperty(TabType::INCOMING_SHARE, mUi->incomingSharesSearch);
    setTabTypeProperty(TabType::BACKUP, mUi->backupsSearch);
    setTabTypeProperty(TabType::RUBBISH, mUi->rubbishSearch);
}

void NodeSelectorSearchController::beginSearch(const QString& text)
{
    hideSearchButtons();
    mNewSearch = true;
    mSearchText = text;
    mUi->stackedWidget->setCurrentWidget(mUi->treeViewPage);
}

void NodeSelectorSearchController::resetSearch()
{
    mSearchScope.reset();
    stopSearch();
    hideSearchButtons();
}

void NodeSelectorSearchController::stopSearch()
{
    mHasRows = false;
}

void NodeSelectorSearchController::setSearchScope(std::optional<TabType> scope)
{
    mSearchScope = scope;
    hideSearchButtons();
}

TabTypes NodeSelectorSearchController::allowedTypes(TabTypes defaultTypes) const
{
    return mSearchScope.has_value() ? TabTypes(mSearchScope.value()) : defaultTypes;
}

bool NodeSelectorSearchController::hasRows() const
{
    return mHasRows;
}

void NodeSelectorSearchController::setHasRows(bool hasRows)
{
    mHasRows = hasRows;
}

bool NodeSelectorSearchController::matchesNodeName(mega::MegaNode* node) const
{
    if (!node)
    {
        return false;
    }

    const auto nodeName = QString::fromUtf8(node->getName());
    return nodeName.contains(mSearchText, Qt::CaseInsensitive);
}

const QString& NodeSelectorSearchController::searchText() const
{
    return mSearchText;
}

void NodeSelectorSearchController::activateMode(TabType type,
                                                NodeSelectorProxyModelSearch* proxyModel,
                                                const std::function<void(TabType)>& applyColumns,
                                                const std::function<void()>& expandResults) const
{
    if (!proxyModel)
    {
        return;
    }

    proxyModel->setMode(type);
    applyColumns(type);
    expandResults();
}

void NodeSelectorSearchController::handleExpandReady(
    NodeSelectorModelSearch* searchModel,
    NodeSelectorProxyModelSearch* proxyModel,
    bool isViewInitialized,
    const std::function<void()>& ensureViewReady,
    const std::function<void(TabType)>& applyColumns,
    const std::function<void()>& expandResults)
{
    if (mNewSearch)
    {
        if (mSearchScope.has_value())
        {
            const auto scopedType = mSearchScope.value();
            applyInitialMode(scopedType, proxyModel, isViewInitialized);
            mNewSearch = false;

            ensureViewReady();
            applyColumns(scopedType);
            expandResults();
            return;
        }

        const auto searchedTypes = searchModel ? searchModel->searchedTypes() : TabTypes{};
        applySearchButtonsVisibility(searchedTypes);

        const auto selectedType = firstAvailableType(searchedTypes);
        if (selectedType != TabType::NONE)
        {
            applyInitialMode(selectedType, proxyModel, isViewInitialized);
        }

        mNewSearch = false;

        ensureViewReady();
        applyColumns(selectedType);
        expandResults();
        return;
    }

    ensureViewReady();
    if (searchModel)
    {
        applySearchButtonsVisibility(searchModel->searchedTypes());
    }
    expandResults();
}

void NodeSelectorSearchController::setTabTypeProperty(TabType type, QWidget* tab) const
{
    if (tab)
    {
        tab->setProperty(TAB_TYPE_PROPERTY, static_cast<int>(type));
    }
}

void NodeSelectorSearchController::hideSearchButtons() const
{
    mUi->searchButtonsWidget->setVisible(false);
}

void NodeSelectorSearchController::applySearchButtonsVisibility(TabTypes searchedTypes) const
{
    mUi->backupsSearch->setVisible(searchedTypes.testFlag(TabType::BACKUP));
    mUi->incomingSharesSearch->setVisible(searchedTypes.testFlag(TabType::INCOMING_SHARE));
    mUi->cloudDriveSearch->setVisible(searchedTypes.testFlag(TabType::CLOUD_DRIVE));
    mUi->rubbishSearch->setVisible(searchedTypes.testFlag(TabType::RUBBISH));

    // Hide the chip bar when a single scope matches: a lone chip offers no real choice.
    const int visibleChips =
        searchedTypes.testFlag(TabType::BACKUP) + searchedTypes.testFlag(TabType::INCOMING_SHARE) +
        searchedTypes.testFlag(TabType::CLOUD_DRIVE) + searchedTypes.testFlag(TabType::RUBBISH);
    mUi->searchButtonsWidget->setVisible(visibleChips > 1);
}

void NodeSelectorSearchController::applyInitialMode(TabType type,
                                                    NodeSelectorProxyModelSearch* proxyModel,
                                                    bool isViewInitialized) const
{
    if (!proxyModel)
    {
        return;
    }

    if (!isViewInitialized)
    {
        TabSelector::applyActionToTabSelectors(mUi->searchButtonsWidget,
                                               [](TabSelector* tab)
                                               {
                                                   tab->blockSignals(true);
                                               });
        TabSelector::selectTabIf(mUi->searchButtonsWidget,
                                 TAB_TYPE_PROPERTY,
                                 static_cast<int>(type));
        TabSelector::applyActionToTabSelectors(mUi->searchButtonsWidget,
                                               [](TabSelector* tab)
                                               {
                                                   tab->blockSignals(false);
                                               });
        proxyModel->setMode(type, false);
    }
    else
    {
        TabSelector::selectTabIf(mUi->searchButtonsWidget,
                                 TAB_TYPE_PROPERTY,
                                 static_cast<int>(type));
    }
}

TabType NodeSelectorSearchController::firstAvailableType(TabTypes searchedTypes) const
{
    if (searchedTypes.testFlag(TabType::CLOUD_DRIVE))
    {
        return TabType::CLOUD_DRIVE;
    }

    if (searchedTypes.testFlag(TabType::INCOMING_SHARE))
    {
        return TabType::INCOMING_SHARE;
    }

    if (searchedTypes.testFlag(TabType::BACKUP))
    {
        return TabType::BACKUP;
    }

    if (searchedTypes.testFlag(TabType::RUBBISH))
    {
        return TabType::RUBBISH;
    }

    return TabType::NONE;
}
