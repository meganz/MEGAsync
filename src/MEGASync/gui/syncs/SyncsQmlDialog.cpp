#include "SyncsQmlDialog.h"

#include "gui/SettingsDialog.h"
#include "DialogOpener.h"

#include <QEvent>

bool SyncsQmlDialog::event(QEvent* event)
{
    if(event->type() == QEvent::Close)
    {
        if(auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setSyncAddButtonEnabled(true);
        }
    }
    else if (event->type() == QEvent::Show)
    {
        if(auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setSyncAddButtonEnabled(false);
        }
    }

    return QmlDialog::event(event);
}
