#ifndef BACKUPSETTINGSELEMENTS_H
#define BACKUPSETTINGSELEMENTS_H

#include "megaapi.h"

#include <QObject>

namespace Ui
{
class OpenBackupsFolder;
}

class SyncSettingsUIBase;

class BackupSettingsElements: public QObject
{
    Q_OBJECT

public:
    explicit BackupSettingsElements(QObject* parent = nullptr);
    ~BackupSettingsElements();

    void initElements(SyncSettingsUIBase* syncSettingsUi);
    void updateUI();
    void retranslateUI();

private slots:
    void onOpenBackupFolderClicked();

private:
    void onMyBackupsFolderHandleSet(mega::MegaHandle h);

    Ui::OpenBackupsFolder* openFolderUi;
    QWidget* mOpenBackupsFolder;
};

#endif // BACKUPSETTINGSELEMENTS_H
