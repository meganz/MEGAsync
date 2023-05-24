#ifndef BACKUPTABLEVIEW_H
#define BACKUPTABLEVIEW_H

#include "syncs/control/SyncSettings.h"
#include "syncs/gui/Twoways/SyncTableView.h"
#include <QObject>
#include <QTableView>

class BackupTableView : public SyncTableView
{
    Q_OBJECT

public:
    BackupTableView(QWidget *parent = nullptr);
    /* To call after model is set */
    void customize();

signals:
    void removeBackup(std::shared_ptr<SyncSettings> backup);
    void openInMEGA(mega::MegaHandle handle);

protected:
    void initTable() override;
    void removeActionClicked(std::shared_ptr<SyncSettings> settings) override;

private slots:
    void onCustomContextMenuRequested(const QPoint& pos) override;
    void onCellClicked(const QModelIndex &index) override;
};

#endif // BACKUPTABLEVIEW_H
