#ifndef SYNC_SETTINGS_ACTIONS_H
#define SYNC_SETTINGS_ACTIONS_H

#include "SyncSettingsActionsBase.h"

// Syncs tab actions (from SyncSettingsQuickWidget).
class SyncSettingsActions: public SyncSettingsActionsBase
{
    Q_OBJECT

    Q_PROPERTY(bool automaticSyncIssueResolverEnabled READ getAutomaticSyncIssueResolverEnabled
                   WRITE setAutomaticSyncIssueResolverEnabled NOTIFY
                       automaticSyncIssueResolverEnabledChanged FINAL)

public:
    explicit SyncSettingsActions(QWidget* hostWidget, QObject* parent = nullptr);

    void addItem() const override;
    void remove(int index) const override;

    Q_INVOKABLE void restoreSyncedFolder(int index);

    bool getAutomaticSyncIssueResolverEnabled() const;
    void setAutomaticSyncIssueResolverEnabled(bool enable);

signals:
    void automaticSyncIssueResolverEnabledChanged();

private:
    bool mAutomaticSyncIssueResolverEnabled;
};

#endif // SYNC_SETTINGS_ACTIONS_H
