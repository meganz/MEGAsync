#ifndef REMOVEBACKUPDIALOG_H
#define REMOVEBACKUPDIALOG_H

#include <QDialog>

#include "model/SyncSettings.h"

namespace Ui {
class RemoveBackupDialog;
}

class RemoveBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveBackupDialog(std::shared_ptr<SyncSetting> backup, QWidget *parent = nullptr);
    ~RemoveBackupDialog();

    std::shared_ptr<SyncSetting> backupToRemove();
    bool alsoRemoveMEGAFolder();

private:
    Ui::RemoveBackupDialog *mUi;
    std::shared_ptr<SyncSetting> mBackup;
};

#endif // REMOVEBACKUPDIALOG_H
