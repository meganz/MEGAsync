#ifndef CHOOSE_MOVE_BACKUP_FOLDER_ERROR_H
#define CHOOSE_MOVE_BACKUP_FOLDER_ERROR_H

#include "megaapi.h"

#include <QDialog>

namespace Ui
{
class ChooseMoveBackupFolderErrorDialog;
}

class ChooseMoveBackupFolderErrorDialog: public QDialog
{
    Q_OBJECT

public:
    explicit ChooseMoveBackupFolderErrorDialog(mega::MegaHandle nonValidTargetFolder,
                                               QWidget* parent = nullptr);
    ~ChooseMoveBackupFolderErrorDialog();

signals:
    void moveBackup(mega::MegaHandle targetFolder);

private slots:
    void onChangeButtonClicked();
    void onConfirmClicked();

private:
    QString getFolderName(mega::MegaHandle handle);

    mega::MegaApi* mMegaApi;
    Ui::ChooseMoveBackupFolderErrorDialog* mUi;
    mega::MegaHandle mTargetFolder;
};

#endif // CHOOSE_MOVE_BACKUP_FOLDER_ERROR_H
