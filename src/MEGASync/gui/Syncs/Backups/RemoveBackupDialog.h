#ifndef REMOVEBACKUPDIALOG_H
#define REMOVEBACKUPDIALOG_H

#include "model/SyncSettings.h"
#include "NodeSelector.h"

#include <QDialog>
#include <QPointer>

namespace Ui {
class RemoveBackupDialog;
}

class RemoveBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveBackupDialog(std::shared_ptr<SyncSettings> backup, QWidget *parent = nullptr);
    ~RemoveBackupDialog();

    std::shared_ptr<SyncSettings> backupToRemove();
    mega::MegaHandle targetFolder();

private slots:
    void OnDeleteSelected();
    void OnMoveSelected();
    void OnChangeButtonClicked();

private:
    mega::MegaApi* mMegaApi;
    Ui::RemoveBackupDialog *mUi;
    std::shared_ptr<SyncSettings> mBackup;
    mega::MegaHandle mTargetFolder;
    QPointer<NodeSelector> mNodeSelector;
};

#endif // REMOVEBACKUPDIALOG_H
