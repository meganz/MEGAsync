#ifndef SETTINGS_TAB_IDS_H
#define SETTINGS_TAB_IDS_H

#include <QObject>

//
// Shared identifiers of the Settings sections. Every deep link into Settings
// (tray, InfoDialog, Transfer Manager, onboarding, notifications, stalled
// issues) names its target through this enum, and SettingsDialog switches on
// it too, so there is a single source of truth and no caller depends on the
// legacy QDialog class. Values 0..7 are the historical tab indexes of that
// dialog and must never change.
//
// Exposed to QML as SettingsTabIds.<NAME> (import SettingsTabIds 1.0),
// registered in QmlManager::registerCommonQmlElements.
namespace SettingsTabIds
{
Q_NAMESPACE

enum Tab
{
    NONE = -1, // keep the current section (MegaApplication::openSettings default)
    GENERAL = 0,
    ACCOUNT = 1,
    SYNCS = 2,
    BACKUP = 3,
    SECURITY = 4,
    FILE_MANAGEMENT = 5, // "Folders" tab of the legacy dialog
    NETWORK = 6,
    NOTIFICATIONS = 7,
    SCREENSHOTS = 8 // section of the redesigned window only; no legacy tab
};
Q_ENUM_NS(Tab)
}

#endif // SETTINGS_TAB_IDS_H
