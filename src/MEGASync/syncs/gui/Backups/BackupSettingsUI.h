#ifndef BACKUPSSETTINGSUI_H
#define BACKUPSSETTINGSUI_H

#include <QWidget>
#include <QPointer>

#include <syncs/gui/SyncSettingsUIBase.h>
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
    QString getFinishWarningIconString() const override;
    QString getFinishIconString() const override;
    QString disableString() const override;

    //Operation failed
    QString getOperationFailTitle() const override;
    QString getOperationFailText(std::shared_ptr<SyncSettings> sync) override;

    //Error adding
    QString getErrorAddingTitle() const override;

    //Error removing
    QString getErrorRemovingTitle() const override;
    QString getErrorRemovingText(std::shared_ptr<mega::MegaError> err) override;

    void changeEvent(QEvent* event);

protected slots:
    void removeSync(std::shared_ptr<SyncSettings> sync) override;

private:
    BackupSettingsElements mElements;
};

#endif // BACKUPSSETTINGSUI_H
