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

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& source_idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    bool isModelEmpty() override;
    QIcon getEmptyIcon() override;
};

class NodeSelectorTreeViewWidgetIncomingShares : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode, QWidget *parent = nullptr);

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void onRootIndexChanged(const QModelIndex& source_idx) override;
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
    void onRootIndexChanged(const QModelIndex& source_idx) override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    QIcon getEmptyIcon() override;
};

class NodeSelectorTreeViewWidgetSearch : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget *parent = nullptr);
    void search(const QString& text);
    std::unique_ptr<NodeSelectorProxyModel> createProxyModel() override;

private slots:
    void onBackupsSearchClicked();
    void onIncomingSharesSearchClicked();
    void onCloudDriveSearchClicked();

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    QIcon getEmptyIcon() override;
    bool isModelEmpty() override;
};

#endif // NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H

