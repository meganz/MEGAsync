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
    void openInMEGA(mega::MegaHandle handle);

protected:
    void initTable() override;

    //Reimplemented methods for contextMenu
    QString getRemoveActionString() override;
};

#endif // BACKUPTABLEVIEW_H
