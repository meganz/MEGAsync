#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include <QDialog>
#include <QInputDialog>
#include <QTreeWidgetItem>
#include <QDir>

#include "sdk/megaapi.h"

namespace Ui {
class NodeSelector;
}

class NodeSelector : public QDialog, public MegaRequestListener
{
    Q_OBJECT

public:
    explicit NodeSelector(MegaApi *megaApi, QWidget *parent = 0);
    ~NodeSelector();

    void nodesReady();
    long long getSelectedFolderHandle();
    virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);

    virtual bool event(QEvent *event);

private:
    Ui::NodeSelector *ui;
    MegaApi *megaApi;
    QIcon folderIcon;
    unsigned long long selectedFolder;
    QTreeWidgetItem *selectedItem;
    MegaRequest *request;
    MegaError *error;

protected:
    void addChildren(QTreeWidgetItem *parentItem, Node *parentNode);


private slots:
    void on_tMegaFolders_itemSelectionChanged();
    void on_bNewFolder_clicked();
};

#endif // NODESELECTOR_H
