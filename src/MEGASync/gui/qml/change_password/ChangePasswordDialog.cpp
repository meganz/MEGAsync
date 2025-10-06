#include "ChangePasswordDialog.h"

#include "DialogOpener.h"

#include <QEvent>

bool ChangePasswordDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::Show)
    {
        if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            // TODO : do we need to do this :???
            // dialog->getDialog()->setChangePasswordButtonEnabled(event->type() == QEvent::Close);
        }
    }

    return QmlDialog::event(event);
}
