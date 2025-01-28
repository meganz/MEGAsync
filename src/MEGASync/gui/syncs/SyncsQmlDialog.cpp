#include "SyncsQmlDialog.h"

#include "DialogOpener.h"
#include "SettingsDialog.h"
#include "SyncReminderNotificationManager.h"

#include <QEvent>

bool SyncsQmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::Show)
    {
        if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setSyncAddButtonEnabled(event->type() == QEvent::Close);
        }

        if (event->type() == QEvent::Close)
        {
            MegaSyncApp->getSyncReminderNotificationManager()->resetClickedTime();
        }
    }

    return QmlDialog::event(event);
}
