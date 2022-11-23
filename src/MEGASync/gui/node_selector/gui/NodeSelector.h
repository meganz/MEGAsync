#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include "QTMegaRequestListener.h"
#include "NodeNameSetterDialog/NewFolderDialog.h"

#include <QDialog>
#include <QItemSelection>
#include <QTimer>

#include <memory>


class NodeSelectorProxyModel;
class NodeSelectorModel;

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
        SHARES,
        VAULT,
    };

    static const int LABEL_ELIDE_MARGIN;
    explicit NodeSelector(int selectMode, QWidget *parent = 0);

    ~NodeSelector();
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption();
    void setSelectedNodeHandle(std::shared_ptr<mega::MegaNode> node = nullptr);
    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    int getSelectMode(){ return mSelectMode;}
    void closeEvent(QCloseEvent* event) override;

protected:
    void changeEvent(QEvent * event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onbOkClicked();
    void onbShowIncomingSharesClicked();
    void onbShowCloudDriveClicked();
    void onbShowBackupsFolderClicked();
    void onOptionSelected(int index);
    void updateNodeSelectorTabs();

#ifdef Q_OS_MAC
    void onTabSelected(int index);
#endif
    void onViewReady(bool isEmpty);

private:
    void processCloseEvent(NodeSelectorProxyModel *proxy, QCloseEvent* event);
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);
    void hideSelector(TabItem item);
#ifdef Q_OS_MAC
    void hideTabSelector(const QString& tabText);
#endif
    void shortCutConnects(int ignoreThis);

    Ui::NodeSelector *ui;
    int mSelectMode;

    mega::MegaApi* mMegaApi;
    bool mManuallyResizedColumn;
};

#endif // NODESELECTOR_H
