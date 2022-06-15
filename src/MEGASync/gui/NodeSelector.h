#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include "QTMegaRequestListener.h"

#include <QDialog>
#include <QItemSelection>
#include <QTimer>

#include <memory>


class MegaItemProxyModel;
class MegaItemModel;

namespace Ui {
class NodeSelector;
class NewFolderDialog;
}

class NodeSelector : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum Type{
        UPLOAD_SELECT = 0,
        DOWNLOAD_SELECT,
        SYNC_SELECT,
        STREAM_SELECT,
    };

    enum TabItem{
        CLOUD_DRIVE = 0,
        SHARES,
        VAULT,
    };

    static const int LABEL_ELIDE_MARGIN;
    static const char* CLD_DRIVE;
    static const char* IN_SHARES;
    static const char* BACKUPS;
    explicit NodeSelector(int selectMode, QWidget *parent = 0);

    ~NodeSelector();
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    void setSelectedNodeHandle(mega::MegaHandle selectedHandle);
    bool getDefaultUploadOption();

public slots:
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    void onDeleteClicked();
    void onGenMEGALinkClicked();

protected:
    void changeEvent(QEvent * event) override;
    void nodesReady();
    void showEvent(QShowEvent* ) override;
    void resizeEvent(QResizeEvent* ) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onItemDoubleClick(const QModelIndex &index);
    void onGoBackClicked();
    void onGoForwardClicked();
    void onbNewFolderClicked();
    void onbOkClicked();
    void onbShowIncomingSharesClicked();
    void onbShowCloudDriveClicked();
    void onbShowBackupsFolderClicked();
    void onTabSelected(int index);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

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

    void checkBackForwardButtons();
    void checkNewFolderButtonVisibility();
    void saveExpandedItems();
    void iterateForSaveExpanded(QList<mega::MegaHandle>& saveList, const QModelIndex& parent = QModelIndex());
    void restoreExpandedItems();
    void restoreExpandedItems(Navigation &nav);
    void iterateForRestore(const QList<mega::MegaHandle> &list, const QModelIndex& parent = QModelIndex());
    bool isAllowedToEnterInIndex(const QModelIndex & idx);
    bool isCloudDrive();
    bool isVault();
    bool isInShares();
    void navigateIntoOperation(Navigation& nav, const QModelIndex& idx);
    void navigateBackwardOperation(Navigation& nav);
    void navigateForwardOperation(Navigation& nav);
    TabItem getSelectedTab();
    bool showBackups();
    void setRootIndex(const QModelIndex& proxy_idx);
    mega::MegaHandle getHandleByIndex(const QModelIndex& idx);
    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    QModelIndex getSelectedIndex();
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);
    Navigation mNavCloudDrive;
    Navigation mNavInShares;
    Navigation mNavVault;

    Ui::NodeSelector *ui;
    Ui::NewFolderDialog *mNewFolderUi;
    QDialog *mNewFolder;
    QTimer mNewFolderErrorTimer;
    int mSelectMode;

    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<MegaItemModel> mModel;
    std::unique_ptr<MegaItemProxyModel> mProxyModel;

    void setupNewFolderDialog();
};

#endif // NODESELECTOR_H
