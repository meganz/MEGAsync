#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QDialog>
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

private slots:
    void on_tCompleted_clicked();
    void on_tDownloads_clicked();
    void on_tUploads_clicked();
    void on_tAllTransfers_clicked();
};

#endif // TRANSFERMANAGER_H
