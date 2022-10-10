#ifndef ADDBACKUPDIALOG_H
#define ADDBACKUPDIALOG_H

#include "control/SyncController.h"
#include "Backups/BackupNameConflictDialog.h"

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

    void setMyBackupsFolder(const QString& folder);
    void setMyBackupsFolderHandle(const mega::MegaHandle& handle);

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
    mega::MegaHandle mMyBackupsHandle;
};

#endif // ADDBACKUPDIALOG_H
