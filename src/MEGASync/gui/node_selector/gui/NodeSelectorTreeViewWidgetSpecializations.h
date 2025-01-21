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

class NodeSelectorTreeViewWidgetCloudDrive : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget *parent = nullptr);
    void itemsRestored(const QList<mega::MegaHandle>& handles);

    void setShowEmptyView(bool newShowEmptyView);

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void modelLoaded() override;
    QIcon getEmptyIcon() override;
    bool showEmptyView() override {return mShowEmptyView;}
    bool isCurrentRootIndexReadOnly() override;

    mega::MegaHandle findMergedSibling(std::shared_ptr<mega::MegaNode> node);

    bool mShowEmptyView = true;
    QList<mega::MegaHandle> mRestoredHandles;
};

class NodeSelectorTreeViewWidgetIncomingShares : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode, QWidget *parent = nullptr);

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    bool isCurrentRootIndexReadOnly() override;
    bool isCurrentSelectionReadOnly() override;
    QIcon getEmptyIcon() override;
};

class NodeSelectorTreeViewWidgetBackups : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode, QWidget *parent = nullptr);

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    bool isCurrentRootIndexReadOnly() override {return true;}
    bool isCurrentSelectionReadOnly() override {return true;}
    QIcon getEmptyIcon() override;
};

class NodeSelectorTreeViewWidgetSearch : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget *parent = nullptr);
    void search(const QString& text);
    void stopSearch();
    std::unique_ptr<NodeSelectorProxyModel> createProxyModel() override;

signals:
    void nodeDoubleClicked(std::shared_ptr<mega::MegaNode> node, bool goToInit);

protected:
    bool newNodeCanBeAdded(mega::MegaNode* node) override;
    QModelIndex getAddedNodeParent(mega::MegaHandle parentHandle) override;

protected slots:
    bool containsIndexToAddOrUpdate(mega::MegaNode* node, const mega::MegaHandle&) override;

private slots:
    void onBackupsSearchClicked();
    void onIncomingSharesSearchClicked();
    void onCloudDriveSearchClicked();
    void onItemDoubleClick(const QModelIndex &index) override;

private:
    void checkAndClick(QToolButton* button);
    void changeButtonsWidgetSizePolicy(bool state);
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    QIcon getEmptyIcon() override;
    void modelLoaded() override;
    bool newFolderBtnCanBeVisisble() override {return false;}
    bool mHasRows;
};

class NodeSelectorTreeViewWidgetRubbish : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode, QWidget *parent = nullptr);
    void setShowEmptyView(bool newShowEmptyView);
    bool isEmpty() const;
    void restoreItems(const QList<mega::MegaHandle>& handles);

signals:
    void itemsRestoreRequested(const QList<mega::MegaHandle>& handles);
    void itemsRestored(const QList<mega::MegaHandle>& handles);

protected:
    void makeCustomConnections() override;

protected slots:
    void onRestoreClicked(const QList<mega::MegaHandle>& handles);

private slots:
    void onRestoreItemsFinished();

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void modelLoaded() override;
    QIcon getEmptyIcon() override;
    bool showEmptyView() override {return mShowEmptyView;}
    bool isCurrentRootIndexReadOnly() override {return true;}
    bool isCurrentSelectionReadOnly() override {return true;}
    bool newFolderBtnCanBeVisisble() override {return false;}

    bool mShowEmptyView = true;
    QList<mega::MegaHandle> mRestoredItems;
};


#endif // NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H

