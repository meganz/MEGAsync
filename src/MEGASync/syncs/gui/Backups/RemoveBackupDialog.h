#ifndef REMOVEBACKUPDIALOG_H
#define REMOVEBACKUPDIALOG_H

#include "AppStatsEvents.h"
#include "megaapi.h"

#include <QDialog>
#include <QPointer>

namespace Ui {
class RemoveBackupDialog;
}

class RemoveBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveBackupDialog(QWidget* parent = nullptr);
    ~RemoveBackupDialog();

signals:
    void removeBackup(mega::MegaHandle targetFolder);

private slots:
    void onDeleteSelected();
    void onMoveSelected();
    void onChangeButtonClicked();
    void onConfirmClicked();

private:
    void onOptionSelected(const AppStatsEvents::EventType eventType,
                          const bool enableMoveContainer);

    mega::MegaApi* mMegaApi;
    Ui::RemoveBackupDialog* mUi;
    mega::MegaHandle mTargetFolder;
};

#endif // REMOVEBACKUPDIALOG_H
