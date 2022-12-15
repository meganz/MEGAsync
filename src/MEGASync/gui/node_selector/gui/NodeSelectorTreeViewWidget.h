#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H

#include "QTMegaRequestListener.h"
#include <megaapi.h>
#include "NodeSelectorLoadingDelegate.h"
#include <ViewLoadingScene.h>

#include <QWidget>
#include <QDebug>
#include <QItemSelectionModel>
#include <memory>


class NodeSelectorProxyModel;
class NodeSelectorModel;
class NodeSelectorModelItem;
class SelectType;
typedef std::unique_ptr<SelectType> SelectTypeUPtr;

namespace Ui {
class NodeSelectorTreeViewWidget;
}


class NodeSelectorTreeViewWidget : public QWidget,  public mega::MegaRequestListener
{
    Q_OBJECT

public:

    static const int LABEL_ELIDE_MARGIN;
    static const int LOADING_VIEW_THRESSHOLD;

    explicit NodeSelectorTreeViewWidget(SelectType *mode, QWidget *parent = nullptr);
    ~NodeSelectorTreeViewWidget();
    mega::MegaHandle getSelectedNodeHandle();
    void setSelectionMode();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    void setSelectedNodeHandle(const mega::MegaHandle &selectedHandle);
    void setFutureSelectedNodeHandle(const mega::MegaHandle &selectedHandle);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption();
    void showDefaultUploadOption(bool show);
    void abort();
    NodeSelectorProxyModel* getProxyModel();

public slots:
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

private slots:
    void onbNewFolderClicked();
    void oncbAlwaysUploadToLocationChanged(bool value);

signals:
    void okBtnClicked();
    void cancelBtnClicked();
    void onViewReady(bool isEmpty);

protected:
    void showEvent(QShowEvent* ) override;
    void resizeEvent(QResizeEvent* ) override;
    void mousePressEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void setTitle(const QString& title);
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);

    Ui::NodeSelectorTreeViewWidget *ui;
    std::unique_ptr<NodeSelectorProxyModel> mProxyModel;


private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onDeleteClicked();
    void onRenameClicked();
    void onGenMEGALinkClicked();
    void onItemDoubleClick(const QModelIndex &index);
    void onGoForwardClicked();
    void onGoBackClicked();
    void onSectionResized();
    void onRowsInserted();
    void onExpandReady();
    void setLoadingSceneVisible(bool visible);
    void onUiBlocked(bool state);

private:
    struct Navigation{
      QList<mega::MegaHandle> expandedHandles = QList<mega::MegaHandle>();
      mega::MegaHandle rootHandle = mega::INVALID_HANDLE;
      QList<mega::MegaHandle> forwardHandles = QList<mega::MegaHandle>();
      QList<mega::MegaHandle> backwardHandles = QList<mega::MegaHandle>();

      void removeFromForward(const mega::MegaHandle& handle);
      void remove(const mega::MegaHandle& handle);

      void appendToBackward(const mega::MegaHandle& handle);
      void appendToForward(const mega::MegaHandle& handle);
    };

    mega::MegaApi* mMegaApi;
    bool mManuallyResizedColumn;
    Navigation mNavigationInfo;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<NodeSelectorModel> mModel;

    bool isAllowedToEnterInIndex(const QModelIndex &idx);
    QModelIndex getSelectedIndex();
    void checkBackForwardButtons();
    void setRootIndex(const QModelIndex& proxy_idx);
    virtual void onRootIndexChanged(const QModelIndex& source_idx){Q_UNUSED(source_idx)};
    mega::MegaHandle getHandleByIndex(const QModelIndex& idx);
    QModelIndex getIndexFromHandle(const mega::MegaHandle &handle);
    void checkNewFolderButtonVisibility();
    virtual QString getRootText() = 0;
    virtual std::unique_ptr<NodeSelectorModel> getModel() = 0;
    virtual bool newFolderBtnVisibleInRoot(){return true;}
    void checkOkButton(const QModelIndexList& selected);

    bool first;
    bool mUiBlocked;
    mega::MegaHandle mNodeHandleToSelect;
    ViewLoadingScene<NodeSelectorLoadingDelegate> mLoadingScene;
    SelectTypeUPtr mSelectType;
    friend class DownloadType;
    friend class SyncType;
    friend class UploadType;
    friend class StreamType;
};

class SelectType
{
public:
    explicit SelectType() = default;
    virtual bool isAllowedToNavigateInside(NodeSelectorModelItem* item);
    virtual void init(NodeSelectorTreeViewWidget* wdg) = 0;
    virtual void checkOkButton(NodeSelectorTreeViewWidget* wdg, const QModelIndexList &selected) = 0;
    virtual void newFolderButtonVisibility(NodeSelectorTreeViewWidget* wdg){Q_UNUSED(wdg)};
};

class DownloadType : public SelectType
{
public:
    explicit DownloadType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    void checkOkButton(NodeSelectorTreeViewWidget* wdg, const QModelIndexList &selected) override;
};

class SyncType : public SelectType
{
public:
    explicit SyncType() = default;
    bool isAllowedToNavigateInside(NodeSelectorModelItem* item) override;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    void newFolderButtonVisibility(NodeSelectorTreeViewWidget* wdg) override;
    void checkOkButton(NodeSelectorTreeViewWidget* wdg, const QModelIndexList &selected) override;
};

class StreamType : public SelectType
{
public:
    explicit StreamType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    void checkOkButton(NodeSelectorTreeViewWidget* wdg, const QModelIndexList &selected) override;
};

class UploadType : public SelectType
{
public:
    explicit UploadType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    void newFolderButtonVisibility(NodeSelectorTreeViewWidget* wdg) override;
    void checkOkButton(NodeSelectorTreeViewWidget* wdg, const QModelIndexList &selected) override;
};

#endif // NODESELECTORTREEVIEWWIDGET_H

