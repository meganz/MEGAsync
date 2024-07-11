#ifndef ADDSYNCFROMUIMANAGER_H
#define ADDSYNCFROMUIMANAGER_H

#include <megaapi.h>

#include <QObject>
#include <QPointer>

class BindFolderDialog;
class SyncController;
class SyncSettings;

class AddSyncFromUiManager : public QObject
{
    Q_OBJECT

public:
    AddSyncFromUiManager() = default;
    ~AddSyncFromUiManager() = default;

    static const AddSyncFromUiManager* addSync(mega::MegaHandle handle = mega::INVALID_HANDLE, bool disableUi = false);
    static const AddSyncFromUiManager* removeSync(mega::MegaHandle handle, QWidget* parent);

signals:
    void syncAdded(mega::MegaHandle remote, const QString& localPath);
    void syncAddingStarted();

private:
    void performAddSync(mega::MegaHandle handle = mega::INVALID_HANDLE, bool disableUi = false);
    void performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent);

private slots:
    void onAddSyncDialogFinished(QPointer<BindFolderDialog> dialog);
};

#endif // ADDSYNCFROMUIMANAGER_H
