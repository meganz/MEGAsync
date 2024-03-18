#ifndef BACKUPSETTINGSELEMENTS_H
#define BACKUPSETTINGSELEMENTS_H

#include <QObject>
#include <megaapi.h>

namespace Ui {
class OpenBackupsFolder;
}

class SyncSettingsUIBase;

class BackupSettingsElements : public QObject
{
    Q_OBJECT

public:
    explicit BackupSettingsElements(QObject *parent = nullptr);
    ~BackupSettingsElements();

    void initElements(SyncSettingsUIBase* syncSettingsUi);
    void updateUI();

private slots:
    void onOpenBackupFolderClicked();

private:
    void onMyBackupsFolderHandleSet(mega::MegaHandle h);

    Ui::OpenBackupsFolder* openFolderUi;
};

#endif // BACKUPSETTINGSELEMENTS_H
