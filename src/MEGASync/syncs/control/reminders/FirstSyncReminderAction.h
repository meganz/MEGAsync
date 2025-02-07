#ifndef FIRST_SYNC_REMINDER_ACTION_H
#define FIRST_SYNC_REMINDER_ACTION_H

#include "SyncReminderAction.h"

class FirstSyncReminderAction: public SyncReminderAction
{
    Q_OBJECT

public:
    using SyncReminderAction::SyncReminderAction;
    ~FirstSyncReminderAction() override = default;

protected:
    virtual QString getTitle() const override;
    virtual QString getMessage() const override;
};

#endif // FIRST_SYNC_REMINDER_ACTION_H
