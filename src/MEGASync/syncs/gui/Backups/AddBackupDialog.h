#ifndef ADDBACKUPDIALOG_H
#define ADDBACKUPDIALOG_H

#include "syncs/control/SyncController.h"
#include "syncs/gui/Backups/BackupNameConflictDialog.h"

#include <QDialog>
#include <QDir>

namespace Ui {
class AddBackupDialog;
}

namespace UserAttributes
{
    class DeviceName;
}

class AddBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBackupDialog(QWidget *parent = nullptr);
    ~AddBackupDialog();

    QString getSelectedFolder();
    QString getBackupName();

private slots:
    void on_changeButton_clicked();
    void onDeviceNameSet(const QString& devName);
    void checkNameConflict();
    void onConflictSolved();

private:
    Ui::AddBackupDialog *mUi;
    QString mSelectedFolder;
    QString mBackupName;
    QString mMyBackupsFolder;
    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
};

#endif // ADDBACKUPDIALOG_H
