#include "BackupsQmlDialog.h"

#include "gui/SettingsDialog.h"
#include "DialogOpener.h"

#include <QEvent>

BackupsQmlDialog::BackupsQmlDialog(QWindow *parent)
    : QmlDialog(parent)
{
}

bool BackupsQmlDialog::event(QEvent* event)
{
    if(event->type() == QEvent::Close)
    {
        if(auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setBackupsAddButtonEnabled(true);
        }
    }
    else if (event->type() == QEvent::Show)
    {
        if(auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setBackupsAddButtonEnabled(false);
        }
    }

    return QmlDialog::event(event);
}
