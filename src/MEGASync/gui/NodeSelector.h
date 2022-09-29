#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include "QTMegaRequestListener.h"
#include "NodeNameSetterDialog/NewFolderDialog.h"

#include <QDialog>
#include <QItemSelection>
#include <QTimer>

#include <memory>


class MegaItemProxyModel;
class MegaItemModel;

namespace Ui {
class NodeSelector;
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
        SHARES
    };

    static const int LABEL_ELIDE_MARGIN;
    static const char* CLD_DRIVE;
    static const char* IN_SHARES;
    explicit NodeSelector(int selectMode, QWidget *parent = 0);

    ~NodeSelector();
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption();
    void setSelectedNodeHandle(const mega::MegaHandle& handle);
    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    int getSelectMode(){ return mSelectMode;}

protected:
    void changeEvent(QEvent * event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onbOkClicked();
    void onbShowIncomingSharesClicked();
    void onbShowCloudDriveClicked();
    void onTabSelected(int index);

private:
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);

    Ui::NodeSelector *ui;
    int mSelectMode;

    mega::MegaApi* mMegaApi;
    bool mManuallyResizedColumn;
};

#endif // NODESELECTOR_H
