#ifndef MULTI_SYNC_REMINDER_ACTION_H
#define MULTI_SYNC_REMINDER_ACTION_H

#include "SyncReminderAction.h"

class MultiSyncReminderAction: public SyncReminderAction
{
    Q_OBJECT

public:
    using SyncReminderAction::SyncReminderAction;
    ~MultiSyncReminderAction() override = default;

protected:
    virtual QString getTitle() const override;
    virtual QString getMessage() const override;
};

#endif // MULTI_SYNC_REMINDER_ACTION_H
