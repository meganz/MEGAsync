#include "PlatformStrings.h"

QString PlatformStrings::openSettings()
{
    return tr("Open preferences");
}

QString PlatformStrings::syncsDisableWarning()
{
    return tr("One or more syncs have been disabled. Go to preferences to enable them again.");
}

QString PlatformStrings::backupsDisableWarning()
{
    return tr("One or more backups have been disabled. Go to preferences to enable them again.");
}

QString PlatformStrings::syncsAndBackupsDisableWarning()
{
    return tr("Some syncs and backups have been disabled. Go to preferences to enable them again.");
}

QString PlatformStrings::cancelSyncsWarning()
{
    return tr("Sync transfers cannot be cancelled individually.\n"
              "Please delete the folder sync from preferences to cancel them.");
}

QString PlatformStrings::movedFileToBin()
{
    return tr("Moved to bin");
}

QString PlatformStrings::fileExplorer()
{
    return tr("Show in Finder");
}

QString PlatformStrings::settings()
{
    return tr("Preferences");
}

QString PlatformStrings::exit()
{
    return tr("Quit");
}
