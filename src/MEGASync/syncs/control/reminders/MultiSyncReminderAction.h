#ifndef MULTI_SYNC_REMINDER_ACTION_H
#define MULTI_SYNC_REMINDER_ACTION_H

#include "SyncReminderAction.h"

class MultiSyncReminderAction: public SyncReminderAction
{
    Q_OBJECT

public:
    using SyncReminderAction::SyncReminderAction;

protected:
    QString getTitle() const override;
    QString getMessage() const override;
    QString getButtonText() const override;
};

#endif // MULTI_SYNC_REMINDER_ACTION_H
