#include "PlatformStrings.h"

QString PlatformStrings::openSettings()
{
    return tr("Open settings");
}

QString PlatformStrings::syncsDisableWarning()
{
    return tr("One or more syncs have been disabled. Go to settings to enable them again.");
}

QString PlatformStrings::cancelSyncsWarning()
{
    return tr("Sync transfers cannot be cancelled individually.\n"
              "Please delete the folder sync from settings to cancel them.");
}
