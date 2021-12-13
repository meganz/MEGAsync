#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include <QDialog>
#include <QInputDialog>
#include <QTreeWidgetItem>
#include <QDir>
#include <QTimer>

#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "QMegaModel.h"
#include "control/SyncController.h"

namespace Ui {
class NodeSelector;
}
namespace Ui {
class NewFolderDialog;
}

class NodeSelector : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum SelectMode
    {
        UPLOAD_SELECT   = 0,
        DOWNLOAD_SELECT = 1,
        SYNC_SELECT     = 2,
        STREAM_SELECT   = 3,
    };

    explicit NodeSelector(mega::MegaApi* megaApi,  SelectMode selectMode, QWidget* parent = nullptr);

    ~NodeSelector();
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    mega::MegaHandle getSelectedFolderHandle();
    void setSelectedFolderHandle(mega::MegaHandle selectedHandle);
    bool getDefaultUploadOption();

public slots:
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e);
    void onCustomContextMenu(const QPoint& point);
    void onDeleteClicked();
    void onGenMEGALinkClicked();

protected:
    void changeEvent(QEvent* event);
    void nodesReady();
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;

private slots:
    void onSelectionChanged(QItemSelection selectedIndexes, QItemSelection);
    void on_bNewFolder_clicked();
    void on_bOk_clicked();
    void onMyBackupsRootDir(mega::MegaHandle handle);

private:
    Ui::NodeSelector* mNodeSelectorUi;
    Ui::NewFolderDialog* mNewFolderUi;
    QDialog* mNewFolderDialog;
    QTimer mNewFolderErrorTimer;

    mega::MegaApi* mMegaApi;
    mega::MegaHandle mSelectedFolder;
    QModelIndex mSelectedItemIndex;
    SelectMode mSelectMode;
    std::shared_ptr<QMegaModel> mRemoteTreeModel;
    mega::MegaHandle mMyBackupsRootDirHandle;
    SyncController mSyncController;

    void setupNewFolderDialog();
};

#endif // NODESELECTOR_H
