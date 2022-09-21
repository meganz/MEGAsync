#include "PlatformStrings.h"

QString PlatformStrings::openSettings()
{
    return tr("Open preferences");
}

QString PlatformStrings::syncsDisableWarning()
{
    return tr("One or more syncs have been disabled. Go to preferences to enable them again.");
}

QString PlatformStrings::cancelSyncsWarning()
{
    return tr("Sync transfers cannot be cancelled individually.\n"
              "Please delete the folder sync from preferences to cancel them.");
}
