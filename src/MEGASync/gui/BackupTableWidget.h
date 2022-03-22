#ifndef BACKUPTABLEWIDGET_H
#define BACKUPTABLEWIDGET_H

#include "model/SyncSettings.h"

#include <QObject>
#include <QTableView>

class BackupTableWidget : public QTableView
{
    Q_OBJECT

public:
    BackupTableWidget(QWidget *parent = nullptr);
    /* To call after model is set */
    void customize();

signals:
    void removeBackup(std::shared_ptr<SyncSetting> backup);
    void openInMEGA(mega::MegaHandle handle);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onCellClicked(const QModelIndex &index);

private:
    void showContextMenu(const QPoint &pos, const QModelIndex index);
};

#endif // BACKUPTABLEWIDGET_H
