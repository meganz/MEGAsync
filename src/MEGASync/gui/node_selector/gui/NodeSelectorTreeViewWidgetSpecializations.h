#ifndef NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H
#define NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H

#include "NodeSelectorTreeViewWidget.h"
#include <QWidget>
#include <QModelIndex>
#include <QLabel>
#include <QToolButton>

#include <memory>

class NodeSelectorProxyModel;
class NodeSelectorModel;

class NodeSelectorTreeViewWidgetCloudDrive : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget *parent = nullptr);

    void setShowEmptyView(bool newShowEmptyView);

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void modelLoaded() override;
    QIcon getEmptyIcon() override;
    bool showEmptyView() override {return mShowEmptyView;}

    bool mShowEmptyView = true;
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
    bool newFolderBtnVisibleInRoot() override {return false;}
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

#endif // NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H

