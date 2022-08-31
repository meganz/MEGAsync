#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H


#include "QTMegaRequestListener.h"
#include <megaapi.h>


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
    static const char* CLD_DRIVE;
    static const char* IN_SHARES;

    explicit NodeSelectorTreeViewWidget(QWidget *parent = nullptr);
    ~NodeSelectorTreeViewWidget();
    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    void setSelectedNodeHandle(const mega::MegaHandle &selectedHandle);
    void newFolderClicked();

public slots:
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

signals:
    void EnableOK(bool value);
    void EnableNewFolder(bool value);
    void SetVisibleNewFolderBtn(bool value);

protected:
    void showEvent(QShowEvent* ) override;
    void resizeEvent(QResizeEvent* ) override;
    void mousePressEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onDeleteClicked();
    void onGenMEGALinkClicked();
    void onItemDoubleClick(const QModelIndex &index);
    void onGoForwardClicked();
    void onGoBackClicked();
    void onSectionResized();
    void onSearchBoxEdited(const QString& text);


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

    Ui::NodeSelectorTreeViewWidget *ui;
    int mSelectMode;
    mega::MegaApi* mMegaApi;
    bool mManuallyResizedColumn;
    Navigation mNavigationInfo;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<MegaItemProxyModel> mProxyModel;
    std::unique_ptr<MegaItemModel> mModel;

    bool isAllowedToEnterInIndex(const QModelIndex &idx);
    QModelIndex getSelectedIndex();
    void checkBackForwardButtons();
    void setRootIndex(const QModelIndex& proxy_idx);
    mega::MegaHandle getHandleByIndex(const QModelIndex& idx);
    QModelIndex getIndexFromHandle(const mega::MegaHandle &handle);
    void checkNewFolderButtonVisibility();
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);
    bool isCloudDrive(){return true;}
};

#endif // NODESELECTORTREEVIEWWIDGET_H
