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
    std::unique_ptr<NodeSelectorModel> getModel() override;
    bool isModelEmpty() override;
};

class NodeSelectorTreeViewWidgetIncomingShares : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode, QWidget *parent = nullptr);

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> getModel() override;
    void onRootIndexChanged(const QModelIndex& source_idx) override;
    bool newFolderBtnVisibleInRoot() override {return false;}
};

class NodeSelectorTreeViewWidgetBackups : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode, QWidget *parent = nullptr);

private:
    QString getRootText() override;
    void onRootIndexChanged(const QModelIndex& source_idx) override;
    std::unique_ptr<NodeSelectorModel> getModel() override;
};

class NodeSelectorTreeViewWidgetSearch : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget *parent = nullptr);
    void search(const QString& text);

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> getModel() override;
};

#endif // NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H

