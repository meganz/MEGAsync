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
    explicit NodeSelector(mega::MegaApi *megaApi, bool rootAllowed, bool sizeWarning, QWidget *parent = 0, bool showFiles = false);

    ~NodeSelector();
    void nodesReady();
    long long getSelectedFolderHandle();

private:
    Ui::NodeSelector *ui;
    mega::MegaApi *megaApi;
    QIcon folderIcon;
    unsigned long long selectedFolder;
    QTreeWidgetItem *selectedItem;
    bool rootAllowed;
    bool sizeWarning;
    bool showFiles;

protected:
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
