#include "BackupsQmlDialog.h"

#include "SettingsDialog.h"
#include "DialogOpener.h"

#include <QEvent>

bool BackupsQmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::Show)
    {
        if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setSyncAddButtonEnabled(event->type() == QEvent::Close,
                                                         SettingsDialog::Tabs::BACKUP_TAB);
        }
    }

    return QmlDialog::event(event);
}
