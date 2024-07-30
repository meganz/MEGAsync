#ifndef BACKUPSSETTINGSUI_H
#define BACKUPSSETTINGSUI_H

#include <QWidget>
#include <QPointer>

#include "SyncSettingsUIBase.h"
#include "BackupSettingsElements.h"

class BackupSettingsUI : public SyncSettingsUIBase
{
    Q_OBJECT

public:
    explicit BackupSettingsUI(QWidget *parent = nullptr);
    ~BackupSettingsUI() override;

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

    void setBackupsTitle();
    void addSyncAfterOverQuotaCheck(const QString& remoteFolder) const override;
    void changeEvent(QEvent* event) override;

protected slots:
    void reqRemoveSync(std::shared_ptr<SyncSettings> backup) override;
    void removeSync(std::shared_ptr<SyncSettings> backup) override;

private:
    BackupSettingsElements mElements;
};

#endif // BACKUPSSETTINGSUI_H
