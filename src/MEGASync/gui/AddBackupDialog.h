#ifndef ADDBACKUPDIALOG_H
#define ADDBACKUPDIALOG_H

#include "control/SyncController.h"

#include <QDialog>
#include <QDir>

namespace Ui {
class AddBackupDialog;
}

class AddBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBackupDialog(QWidget *parent = nullptr);
    ~AddBackupDialog();

    void setMyBackupsFolder(const QString& folder);
    QString getSelectedFolder();

public slots:
    void on_changeButton_clicked();
    void onDeviceNameSet(const QString& devName);

private:
    Ui::AddBackupDialog *mUi;
    QString mSelectedFolder;
    QString mMyBackupsFolder;
    SyncController mSyncController;
    QString mDeviceName;
};

#endif // ADDBACKUPDIALOG_H
