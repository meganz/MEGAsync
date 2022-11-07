#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H


#include "QTMegaRequestListener.h"
#include <megaapi.h>
#include <NodeSelectorLoadingDelegate.h>
#include <ViewLoadingScene.h>

#include <QWidget>
#include <QItemSelectionModel>
#include <memory>


class MegaItemProxyModel;
class MegaItemModel;

namespace Ui {
class NodeSelectorTreeViewWidget;
}

class NodeSelectorTreeViewWidget : public QWidget,  public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum Type{
        UPLOAD_SELECT = 0,
        DOWNLOAD_SELECT,
        SYNC_SELECT,
        STREAM_SELECT,
    };

    static const int LABEL_ELIDE_MARGIN;
    static const int LOADING_VIEW_THRESSHOLD;
    static const char* CLD_DRIVE;
    static const char* IN_SHARES;

    explicit NodeSelectorTreeViewWidget(QWidget *parent = nullptr);
    ~NodeSelectorTreeViewWidget();
    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    void setSelectedNodeHandle(const mega::MegaHandle &selectedHandle);
    void setFutureSelectedNodeHandle(const mega::MegaHandle &selectedHandle);
    void setSelectionMode(int selectionMode);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption();
    void showDefaultUploadOption(bool show);
    void abort();
    MegaItemProxyModel* getProxyModel();

public slots:
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

private slots:
    void onbNewFolderClicked();
    void oncbAlwaysUploadToLocationChanged(bool value);

signals:
    void okBtnClicked();
    void cancelBtnClicked();

protected:
    void showEvent(QShowEvent* ) override;
    void resizeEvent(QResizeEvent* ) override;
    void mousePressEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void setTitle(const QString& title);
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);

    Ui::NodeSelectorTreeViewWidget *ui;
    std::unique_ptr<MegaItemProxyModel> mProxyModel;


private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onDeleteClicked();
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

    int mSelectMode;
    mega::MegaApi* mMegaApi;
    bool mManuallyResizedColumn;
    Navigation mNavigationInfo;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<MegaItemModel> mModel;

    bool isAllowedToEnterInIndex(const QModelIndex &idx);
    QModelIndex getSelectedIndex();
    void checkBackForwardButtons();
    void setRootIndex(const QModelIndex& proxy_idx);
    virtual void setRootIndex_Reimplementation(const QModelIndex& source_idx){Q_UNUSED(source_idx)};
    mega::MegaHandle getHandleByIndex(const QModelIndex& idx);
    QModelIndex getIndexFromHandle(const mega::MegaHandle &handle);
    void checkNewFolderButtonVisibility();
    virtual QString getRootText() = 0;
    virtual std::unique_ptr<MegaItemModel> getModel() = 0;
    virtual bool newFolderBtnVisibleInRoot(){return true;}
    void checkOkButton(const QModelIndexList& selected);

    bool first;
    bool mUiBlocked;
    mega::MegaHandle mNodeHandleToSelect;
    ViewLoadingScene<NodeSelectorLoadingDelegate> mLoadingScene;
};

class NodeSelectorTreeViewWidgetCloudDrive : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetCloudDrive(QWidget *parent = nullptr);

private:
    QString getRootText() override;
    void setRootIndex_Reimplementation(const QModelIndex& source_idx) override;
    std::unique_ptr<MegaItemModel> getModel() override;
};

class NodeSelectorTreeViewWidgetIncomingShares : public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetIncomingShares(QWidget *parent = nullptr);

private:
    QString getRootText() override;
    std::unique_ptr<MegaItemModel> getModel() override;
    void setRootIndex_Reimplementation(const QModelIndex& source_idx) override;
    bool newFolderBtnVisibleInRoot() override {return false;}
};
#endif // NODESELECTORTREEVIEWWIDGET_H

