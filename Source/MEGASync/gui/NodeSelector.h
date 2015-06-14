#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include <QDialog>
#include <QInputDialog>
#include <QTreeWidgetItem>
#include <QDir>

#include "megaapi.h"
#include "QTMegaRequestListener.h"

namespace Ui {
class NodeSelector;
}

class NodeSelector : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum { UPLOAD_SELECT = 0, DOWNLOAD_SELECT, SYNC_SELECT};

    explicit NodeSelector(mega::MegaApi *megaApi,  int selectMode, QWidget *parent = 0);

    ~NodeSelector();
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    long long getSelectedFolderHandle();
    void setSelectedFolderHandle(long long selectedHandle);
    bool getDefaultUploadOption();

private:
    Ui::NodeSelector *ui;
    mega::MegaApi *megaApi;
    QIcon folderIcon;
    unsigned long long selectedFolder;
    QTreeWidgetItem *selectedItem;
    int selectMode;

protected:
    void nodesReady();
    void addChildren(QTreeWidgetItem *parentItem, mega::MegaNode *parentNode);
    mega::QTMegaRequestListener *delegateListener;

public slots:
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

protected:
    void changeEvent(QEvent * event);

private slots:
    void on_tMegaFolders_itemSelectionChanged();
    void on_bNewFolder_clicked();
    void on_bOk_clicked();
};

#endif // NODESELECTOR_H
