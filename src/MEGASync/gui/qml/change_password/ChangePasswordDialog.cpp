#include "ChangePasswordDialog.h"

#include "DialogOpener.h"
#include "SettingsDialog.h"

#include <QEvent>

bool ChangePasswordDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::Show)
    {
        if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setChangePasswordEnabled(event->type() == QEvent::Close);
        }
    }

    return QmlDialog::event(event);
}
