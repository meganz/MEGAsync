#include "SyncsQmlDialog.h"

#include "DialogOpener.h"
#include "SettingsDialog.h"
#include "SyncInfo.h"

#include <QEvent>

bool SyncsQmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::Show)
    {
        if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setSyncAddButtonEnabled(
                event->type() == QEvent::Close,
                isBackup() ? SettingsDialog::Tabs::BACKUP_TAB : SettingsDialog::Tabs::SYNCS_TAB);
        }

        if (!isBackup() && event->type() == QEvent::Close)
        {
            emit MegaSyncApp->syncsDialogClosed();
        }
    }

    return QmlDialog::event(event);
}

bool SyncsQmlDialog::isBackup() const
{
    return mIsBackup;
}

void SyncsQmlDialog::setIsBackup(bool newIsBackup)
{
    mIsBackup = newIsBackup;
}
