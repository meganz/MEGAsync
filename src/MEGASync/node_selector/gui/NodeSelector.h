#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include "ButtonIconManager.h"
#include "NodeSelectorTreeViewWidget.h"

#include <QDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QItemSelection>
#include <QTimer>

#include <memory>
#include <optional>

class NodeSelectorProxyModel;
class NodeSelectorModel;
struct NodeSelectorMergeInfo;
class NodeSelectorTreeViewWidgetCloudDrive;
class NodeSelectorTreeViewWidgetIncomingShares;
class NodeSelectorTreeViewWidgetBackups;
class NodeSelectorTreeViewWidgetSearch;
class NodeSelectorTreeViewWidgetRubbish;
class DuplicatedNodeDialog;
struct ConflictTypes;

struct MessageInfo;

namespace mega
{
class MegaApi;
}

namespace Ui
{
class NodeSelector;
}

class NodeSelector: public QDialog, public mega::MegaListener
{
    Q_OBJECT

public:
    enum TabItem
    {
        CLOUD_DRIVE = 0,
        SHARES,
        BACKUPS,
        RUBBISH,
        SEARCH
    };
    Q_ENUM(TabItem)

    explicit NodeSelector(SelectTypeSPtr selectType, QWidget* parent = 0);

    ~NodeSelector();
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption();
    void setSelectedNodeHandle(std::shared_ptr<mega::MegaNode> node = nullptr);
    mega::MegaHandle findIndexToMoveItem();
    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    void closeEvent(QCloseEvent* event) override;
    static void showNotFoundNodeMessageBox();

public slots:
    void onUpdateLoadingMessage(std::shared_ptr<MessageInfo> message);

protected:
    bool event(QEvent* event) override;
    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent*) override;
    void addBackupsView();
    std::shared_ptr<mega::MegaNode> getSelectedNode();
    void initSpecialisedWidgets();
    bool eventFilter(QObject* obj, QEvent* event) override;

    virtual void onRequestFinish(mega::MegaApi* api,
                                 mega::MegaRequest* request,
                                 mega::MegaError* e) override
    {}

    void onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList* nodes) override;

    enum class IncreaseOrDecrease
    {
        INCREASE,
        DECREASE
    };

    void performItemsToBeMoved(const QList<mega::MegaHandle>& handles,
                               IncreaseOrDecrease type,
                               bool blockSource,
                               bool blockTarget);

    // Create specialised widgets
    virtual void createSpecialisedWidgets() = 0;
    void addCloudDrive();
    NodeSelectorTreeViewWidgetCloudDrive* mCloudDriveWidget;
    void addIncomingShares();
    NodeSelectorTreeViewWidgetIncomingShares* mIncomingSharesWidget;
    void addBackups();
    NodeSelectorTreeViewWidgetBackups* mBackupsWidget;
    void addSearch();
    NodeSelectorTreeViewWidgetSearch* mSearchWidget;
    void addRubbish();
    NodeSelectorTreeViewWidgetRubbish* mRubbishWidget;

    mega::MegaApi* mMegaApi;
    Ui::NodeSelector* ui;
    SelectTypeSPtr mSelectType;

protected slots:

    virtual void onCustomBottomButtonClicked(uint id)
    {
        Q_UNUSED(id)
    }

    virtual void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int actionType);
    virtual void onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int type);

    virtual void onItemsAboutToBeRestored(const QSet<mega::MegaHandle>&) {}

    virtual void onItemAboutToBeReplaced(mega::MegaHandle) {}

    virtual void onItemsAboutToBeMerged(const QList<std::shared_ptr<NodeSelectorMergeInfo>>&, int)
    {}

    virtual void onItemsAboutToBeMergedFailed(const QList<std::shared_ptr<NodeSelectorMergeInfo>>&,
                                              int)
    {}

    void onbShowCloudDriveClicked();
    void onbShowIncomingSharesClicked();

private slots:
    void onbShowSearchClicked();
    void onbOkClicked();
    void onbShowRubbishClicked();
    void onbShowBackupsFolderClicked();
    void onOptionSelected(int index);
    void updateNodeSelectorTabs();
    void onSearch(const QString& text);
    void on_tClearSearchResultNS_clicked();
    void onCurrentWidgetChanged(int index);
    void onShowDuplicatedNodeDialog(QPointer<DuplicatedNodeDialog>);
    void performNodeSelection();

private:
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);
    void setToggledStyle(TabItem item);
    void setAllFramesItsOnProperty();
    virtual void onOkButtonClicked() = 0;
    void shortCutConnects(int ignoreThis);

    std::optional<TabItem> selectedNodeTab();

    ButtonIconManager mButtonIconManager;
    QGraphicsDropShadowEffect* mShadowTab;
    QMap<TabItem, QFrame*> mTabFramesToggleGroup;
    std::unique_ptr<mega::QTMegaListener> mDelegateListener;

    NodeSelectorTreeViewWidget* getTreeViewWidget(int page) const;
    NodeSelectorTreeViewWidget* getTreeViewWidget(QObject* object) const;
    NodeSelectorTreeViewWidget* getCurrentTreeViewWidget() const;

    bool mManuallyResizedColumn;
    bool mInitialised;

    std::shared_ptr<mega::MegaNode> mNodeToBeSelected;

    // Duplicated node details
    std::shared_ptr<ConflictTypes> mDuplicatedConflicts;
    std::optional<int> mDuplicatedType;
    NodeSelectorModel* mDuplicatedModel;

    // Loading view
    QList<NodeSelectorTreeViewWidget*> mSourceWids;
    NodeSelectorTreeViewWidget* mTargetWid;
};

#endif // NODESELECTOR_H
