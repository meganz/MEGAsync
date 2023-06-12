#ifndef BACKUPSSETTINGSUI_H
#define BACKUPSSETTINGSUI_H

#include <QWidget>
#include <QPointer>

#include <SyncSettingsUIBase.h>
#include <syncs/gui/Backups/BackupTableView.h>
#include <syncs/gui/Backups/BackupSettingsElements.h>

class BackupSettingsUI : public SyncSettingsUIBase
{
    Q_OBJECT

public:
    explicit BackupSettingsUI(QWidget *parent = nullptr);
    ~BackupSettingsUI() override;

    void addButtonClicked(mega::MegaHandle megaFolderHandle = mega::INVALID_HANDLE) override;

protected:
    QString getFinishWarningIconString() override;
    QString getFinishIconString() override;
    QString typeString() override;

    void changeEvent(QEvent* event);

protected slots:
    void removeSync(std::shared_ptr<SyncSettings> sync) override;

private:
    BackupSettingsElements mElements;
};

#endif // BACKUPSSETTINGSUI_H
