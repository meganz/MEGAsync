#include "PlatformStrings.h"

QString PlatformStrings::openSettings()
{
    return tr("Open settings");
}

QString PlatformStrings::syncsDisableWarning()
{
    return tr("One or more syncs have been disabled. Go to settings to enable them again.");
}

QString PlatformStrings::backupsDisableWarning()
{
    return tr("One or more backups have been disabled. Go to settings to enable them again.");
}

QString PlatformStrings::syncsAndBackupsDisableWarning()
{
    return tr("Some syncs and backups have been disabled. Go to settings to enable them again.");
}

QString PlatformStrings::cancelSyncsWarning()
{
    return tr("Sync transfers cannot be cancelled individually.\n"
              "Please delete the folder sync from settings to cancel them.");
}
