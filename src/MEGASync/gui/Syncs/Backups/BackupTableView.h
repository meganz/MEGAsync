#ifndef BACKUPTABLEVIEW_H
#define BACKUPTABLEVIEW_H

#include "model/SyncSetting.h"
#include "Syncs/Twoways/SyncTableView.h"
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
    void removeBackup(std::shared_ptr<SyncSetting> backup);
    void openInMEGA(mega::MegaHandle handle);

protected:
    void initTable() override;

private slots:
    void onCustomContextMenuRequested(const QPoint& pos) override;
    void onCellClicked(const QModelIndex &index) override;

private:
    void showContextMenu(const QPoint &pos, const QModelIndex index);
};

#endif // BACKUPTABLEVIEW_H
