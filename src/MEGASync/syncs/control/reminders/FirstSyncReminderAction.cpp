#include "FirstSyncReminderAction.h"

QString FirstSyncReminderAction::getTitle() const
{
    return tr("You’re almost done");
}

QString FirstSyncReminderAction::getMessage() const
{
    return tr("Set up your first sync to get the most out of the desktop app");
}
