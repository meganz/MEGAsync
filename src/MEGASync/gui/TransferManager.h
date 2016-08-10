#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QDialog>
#include <QMenu>
#include "megaapi.h"

namespace Ui {
class TransferManager;
}

class TransferManager : public QDialog
{
    Q_OBJECT

public:
    explicit TransferManager(mega::MegaApi *megaApi, QWidget *parent = 0);
    ~TransferManager();

private:
    Ui::TransferManager *ui;
    mega::MegaApi *megaApi;
    QMenu *addMenu;
    QAction *settingsAction;
    QAction *importLinksAction;
    QAction *uploadAction;
    QAction *downloadAction;

    void createAddMenu();

private slots:
    void on_tCompleted_clicked();
    void on_tDownloads_clicked();
    void on_tUploads_clicked();
    void on_tAllTransfers_clicked();
    void on_bAdd_clicked();
    void on_bClose_clicked();
    void on_bPause_clicked();
    void on_bClearAll_clicked();
};

#endif // TRANSFERMANAGER_H
