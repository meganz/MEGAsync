#ifndef NODESELECTORSEARCHCONTROLLER_H
#define NODESELECTORSEARCHCONTROLLER_H

#include "NodeSelectorTabTypes.h"

#include <QString>
#include <QWidget>

#include <functional>
#include <optional>

namespace mega
{
class MegaNode;
}

namespace Ui
{
class NodeSelectorTreeViewWidget;
}

class NodeSelectorModelSearch;
class NodeSelectorProxyModelSearch;

class NodeSelectorSearchController
{
public:
    explicit NodeSelectorSearchController(Ui::NodeSelectorTreeViewWidget* ui);

    void beginSearch(const QString& text);
    void resetSearch();
    void stopSearch();
    void setSearchScope(std::optional<TabType> scope);
    TabTypes allowedTypes(TabTypes defaultTypes) const;

    bool hasRows() const;
    void setHasRows(bool hasRows);
    bool matchesNodeName(mega::MegaNode* node) const;
    const QString& searchText() const;

    void activateMode(TabType type,
                      NodeSelectorProxyModelSearch* proxyModel,
                      const std::function<void(TabType)>& applyColumns,
                      const std::function<void()>& expandResults) const;

    void handleExpandReady(NodeSelectorModelSearch* searchModel,
                           NodeSelectorProxyModelSearch* proxyModel,
                           bool isViewInitialized,
                           const std::function<void()>& ensureViewReady,
                           const std::function<void(TabType)>& applyColumns,
                           const std::function<void()>& expandResults);

private:
    void initialize();
    void setTabTypeProperty(TabType type, QWidget* tab) const;
    void hideSearchButtons() const;
    void applySearchButtonsVisibility(TabTypes searchedTypes) const;
    void applyInitialMode(TabType type,
                          NodeSelectorProxyModelSearch* proxyModel,
                          bool isViewInitialized) const;
    TabType firstAvailableType(TabTypes searchedTypes) const;

    Ui::NodeSelectorTreeViewWidget* mUi;
    bool mHasRows = false;
    bool mNewSearch = true;
    QString mSearchText;
    std::optional<TabType> mSearchScope;
};

#endif // NODESELECTORSEARCHCONTROLLER_H
