#ifndef ADDSYNCFROMUIMANAGER_H
#define ADDSYNCFROMUIMANAGER_H

#include <megaapi.h>

#include <QObject>
#include <QPointer>

class BindFolderDialog;
class SyncController;

class AddSyncFromUiManager : public QObject
{
    Q_OBJECT

public:
    AddSyncFromUiManager() = default;
    ~AddSyncFromUiManager();

    void addSync(mega::MegaHandle handle = mega::INVALID_HANDLE, bool disableUi = false);

signals:
    void syncAdded(mega::MegaHandle remote, const QString& localPath);

private slots:
    void onAddSyncDialogFinished(QPointer<BindFolderDialog> dialog);

private:
    QPointer<BindFolderDialog> mAddSyncDialog;
    QPointer<SyncController> mSyncController;
};

#endif // ADDSYNCFROMUIMANAGER_H
