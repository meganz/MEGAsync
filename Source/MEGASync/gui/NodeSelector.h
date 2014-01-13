#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include <QDialog>
#include <QInputDialog>
#include <QTreeWidgetItem>
#include <QDir>

#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaRequestListener.h"

namespace Ui {
class NodeSelector;
}

class NodeSelector : public QDialog, public MegaRequestListener
{
    Q_OBJECT

public:
    explicit NodeSelector(MegaApi *megaApi, bool rootAllowed, bool sizeWarning, QWidget *parent = 0);
    ~NodeSelector();
    void nodesReady();
    long long getSelectedFolderHandle();

private:
    Ui::NodeSelector *ui;
    MegaApi *megaApi;
    QIcon folderIcon;
    unsigned long long selectedFolder;
    QTreeWidgetItem *selectedItem;
    bool rootAllowed;
    bool sizeWarning;

protected:
    void addChildren(QTreeWidgetItem *parentItem, Node *parentNode);
	QTMegaRequestListener *delegateListener;

public slots:
	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);

protected:
    void changeEvent(QEvent * event);

private slots:
    void on_tMegaFolders_itemSelectionChanged();
    void on_bNewFolder_clicked();
    void on_bOk_clicked();
};

#endif // NODESELECTOR_H
