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
    enum { UPLOAD_SELECT = 0, DOWNLOAD_SELECT, SYNC_SELECT, STREAM_SELECT};

    explicit NodeSelector(mega::MegaApi *megaApi,  int selectMode, QWidget *parent = 0);

    ~NodeSelector();
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    mega::MegaHandle getSelectedFolderHandle();
    void setSelectedFolderHandle(mega::MegaHandle selectedHandle);
    bool getDefaultUploadOption();

public slots:
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
    void onCustomContextMenu(const QPoint &);
    void onDeleteClicked();
    void onGenMEGALinkClicked();

protected:
    void changeEvent(QEvent * event);
    void nodesReady();
    mega::QTMegaRequestListener *delegateListener;

private slots:
    void onSelectionChanged(QItemSelection,QItemSelection);
    void on_bNewFolder_clicked();
    void on_bOk_clicked();

private:
    Ui::NodeSelector *ui;
    Ui::NewFolderDialog *newFolderUi;
    QDialog *newFolder;
    QTimer newFolderErrorTimer;

    mega::MegaApi *megaApi;
    QIcon folderIcon;
    unsigned long long selectedFolder;
    QModelIndex selectedItem;
    int selectMode;
    QMegaModel *model;

    void setupNewFolderDialog();
};

#endif // NODESELECTOR_H
