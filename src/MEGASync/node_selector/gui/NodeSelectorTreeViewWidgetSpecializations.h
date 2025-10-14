#ifndef NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H
#define NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H

#include "NodeSelectorTreeViewWidget.h"

#include <QLabel>
#include <QModelIndex>
#include <QToolButton>
#include <QWidget>

#include <memory>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class RestoreNodeManager;
class TabSelector;

class NodeSelectorTreeViewWidgetCloudDrive: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget* parent = nullptr);

    void setShowEmptyView(bool newShowEmptyView);

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void modelLoaded() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;

    bool showEmptyView() override
    {
        return mShowEmptyView;
    }
    bool isCurrentRootIndexReadOnly() override;

    mega::MegaHandle findMergedSibling(std::shared_ptr<mega::MegaNode> node);

    bool mShowEmptyView = true;
    QList<mega::MegaHandle> mRestoredHandles;
};

class NodeSelectorTreeViewWidgetIncomingShares: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode,
                                                      QWidget* parent = nullptr);

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    bool isCurrentRootIndexReadOnly() override;
    bool isSelectionReadOnly(const QModelIndexList& selection) override;
    bool isCurrentSelectionReadOnly() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;
};

class NodeSelectorTreeViewWidgetBackups: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode, QWidget* parent = nullptr);

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;

    bool isCurrentRootIndexReadOnly() override
    {
        return true;
    }

    bool isCurrentSelectionReadOnly() override
    {
        return true;
    }

    bool isSelectionReadOnly(const QModelIndexList&) override
    {
        return true;
    }

    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;
};

class NodeSelectorTreeViewWidgetSearch: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget* parent = nullptr);
    void search(const QString& text);
    void stopSearch();
    std::unique_ptr<NodeSelectorProxyModel> createProxyModel() override;
    bool isCurrentRootIndexReadOnly() override;

    std::shared_ptr<RestoreNodeManager> getRestoreManager() const;

public slots:
    void resetMovingNumber();
    void modelLoaded() override;

signals:
    void nodeDoubleClicked(std::shared_ptr<mega::MegaNode> node, bool goToInit);
    void searchCounterChanged();

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;
    QModelIndex getAddedNodeParent(mega::MegaHandle parentHandle) override;
    void makeCustomConnections() override;

protected slots:
    NodeState getNodeOnModelState(const QModelIndex& index, mega::MegaNode* node) override;

private slots:
    void onBackupsSearchClicked();
    void onIncomingSharesSearchClicked();
    void onCloudDriveSearchClicked();
    void onRubbishSearchClicked();
    void onItemDoubleClick(const QModelIndex& index) override;

private:
    void checkSearchButtonsVisibility();
    void checkAndClick(TabSelector* tab);
    void changeButtonsWidgetSizePolicy(bool state);
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;

    bool newFolderBtnCanBeVisisble() override
    {
        return false;
    }
    bool mHasRows;
    QString mSearchStr;

    std::shared_ptr<RestoreNodeManager> mRestoreManager;
};

class NodeSelectorTreeViewWidgetRubbish: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode, QWidget* parent = nullptr);
    void setShowEmptyView(bool newShowEmptyView);
    bool isEmpty() const;

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;
    void makeCustomConnections() override;

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void modelLoaded() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;

    bool showEmptyView() override
    {
        return mShowEmptyView;
    }

    bool isCurrentRootIndexReadOnly() override
    {
        return true;
    }

    bool isCurrentSelectionReadOnly() override
    {
        return true;
    }

    bool isSelectionReadOnly(const QModelIndexList&) override
    {
        return true;
    }

    bool newFolderBtnCanBeVisisble() override
    {
        return false;
    }

    bool mShowEmptyView = true;

    std::shared_ptr<RestoreNodeManager> mRestoreManager;
};

#endif // NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H
