#include "ChangePasswordDialog.h"

#include "DialogOpener.h"
#include "SettingsDialog.h"

#include <QEvent>

bool ChangePasswordDialog::event(QEvent* event)
{
    // While the change-password request is in flight the dialog is locked:
    // the close button (and any other close request) is ignored until the
    // change resolves (SNC-2109). `busy` is declared on the QML root object;
    // QQuickWindow::closing is a revisioned signal not exposed through this
    // registered type, so the guard lives here instead of in QML.
    if (event->type() == QEvent::Close && property("busy").toBool())
    {
        event->ignore();
        return true;
    }

    if (event->type() == QEvent::Close || event->type() == QEvent::Show)
    {
        if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setChangePasswordEnabled(event->type() == QEvent::Close);
        }
    }

    return QmlDialog::event(event);
}
