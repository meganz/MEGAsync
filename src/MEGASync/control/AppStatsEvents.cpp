#include "AppStatsEvents.h"

// Deprecated are not displayed
QHash<AppStatsEvents::EventTypes, const char*> AppStatsEvents::statsMap = {
    { AppStatsEvents::EVENT_1ST_START, "MEGAsync first start" },
    { AppStatsEvents::EVENT_1ST_SYNC, "MEGAsync first sync" },
    { AppStatsEvents::EVENT_1ST_SYNCED_FILE, "MEGAsync first synced file" },
    { AppStatsEvents::EVENT_1ST_WEBCLIENT_DL, "MEGAsync first webclient download" },
    { AppStatsEvents::EVENT_UNINSTALL_STATS, "{\"it\":%1,\"act\":%2,\"lt\":%3}" },
    { AppStatsEvents::EVENT_ACC_CREATION_START, "MEGAsync account creation start" },
    { AppStatsEvents::EVENT_PRO_REDIRECT, "Redirection to PRO" },
    { AppStatsEvents::EVENT_MEM_USAGE, "%1 %2 %3"},
    { AppStatsEvents::EVENT_UPDATE, "MEGAsync update" },
    { AppStatsEvents::EVENT_UPDATE_OK, "MEGAsync updated OK" },
    { AppStatsEvents::EVENT_DUP_FINISHED_TRSF, "Duplicated finished transfer: %1" },
    { AppStatsEvents::EVENT_OVER_STORAGE_DIAL, "Overstorage dialog shown" },
    { AppStatsEvents::EVENT_OVER_STORAGE_NOTIF, "Overstorage notification shown" },
    { AppStatsEvents::EVENT_OVER_STORAGE_MSG, "Overstorage warning shown" },
    { AppStatsEvents::EVENT_ALMOST_OVER_STORAGE_MSG, "Almost overstorage warning shown" },
    { AppStatsEvents::EVENT_ALMOST_OVER_STORAGE_NOTIF, "Almost overstorage notification shown" },
    { AppStatsEvents::EVENT_MAIN_DIAL_WHILE_OVER_QUOTA, "Main dialog shown while overquota" },
    { AppStatsEvents::EVENT_MAIN_DIAL_WHILE_ALMOST_OVER_QUOTA, "Main dialog shown while almost overquota" },
    { AppStatsEvents::EVENT_RED_LIGHT_USED_STORAGE_MISMATCH, "Red light does not match used storage" },
    { AppStatsEvents::EVENT_TRSF_OVER_QUOTA_DIAL, "Transfer over quota dialog shown" },
    { AppStatsEvents::EVENT_TRSF_OVER_QUOTA_NOTIF, "Transfer over quota os notification shown" },
    { AppStatsEvents::EVENT_TRSF_OVER_QUOTA_MSG, "Transfer over quota ui message shown" },
    { AppStatsEvents::EVENT_TRSF_ALMOST_OVER_QUOTA_MSG, "Transfer almost over quota ui message shown" },
    { AppStatsEvents::EVENT_PAYWALL_NOTIF, "Paywall notification shown" },
    { AppStatsEvents::EVENT_SYNC_ADD_FAIL_API_EACCESS, "Sync addition fails with API_EACCESS" },
    { AppStatsEvents::EVENT_TRSF_ALMOST_OVERQUOTA_NOTIF, "Transfer almost over quota os notification shown" },
    { AppStatsEvents::EVENT_1ST_BACKUP, "MEGAsync first backup" },
    { AppStatsEvents::EVENT_1ST_BACKED_UP_FILE, "MEGAsync first backed-up file" },
    { AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_MANUALLY, "Name conflict issue solved manually" },
    { AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_AUTOMATICALLY, "Name conflict issue solved automatically" },
    { AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_SEMI_AUTOMATICALLY, "Name conflict issue solved semi-automatically" },
    { AppStatsEvents::EVENT_SI_LOCALREMOTE_SOLVED_MANUALLY, "Local/Remote issue solved manually" },
    { AppStatsEvents::EVENT_SI_LOCALREMOTE_SOLVED_AUTOMATICALLY, "Local/Remote issue solved automatically" },
    { AppStatsEvents::EVENT_SI_LOCALREMOTE_SOLVED_SEMI_AUTOMATICALLY, "Local/Remote issue solved semi-automatically" },
    { AppStatsEvents::EVENT_SI_IGNORE_SOLVED_MANUALLY, "Issue ignored manually" },
    { AppStatsEvents::EVENT_SI_STALLED_ISSUE_RECEIVED, "Stalled issue received: Type %1" },
    { AppStatsEvents::EVENT_SI_IGNORE_ALL_SYMLINK, "All symlink ignored" },
    { AppStatsEvents::EVENT_SI_SMART_MODE_FIRST_SELECTED, "Smart mode selected by default" },
    { AppStatsEvents::EVENT_SI_ADVANCED_MODE_FIRST_SELECTED, "Advanced mode selected by default" },
    { AppStatsEvents::EVENT_SI_CHANGE_TO_SMART_MODE, "Smart mode selected" },
    { AppStatsEvents::EVENT_SI_CHANGE_TO_ADVANCED_MODE, "Advanced mode selected" },
    { AppStatsEvents::EVENT_SI_FINGERPRINT_MISSING_SOLVED_MANUALLY, "Cloud fingerprint missing solved manually" },
    { AppStatsEvents::EVENT_DAILY_ACTIVE_USER, "Daily Active Users (DAU) - acctype: %1" },
    { AppStatsEvents::EVENT_MONTHLY_ACTIVE_USER, "Monthly Active Users (MAU) - acctype: %1" }
};

const char* AppStatsEvents::getEventMessage(EventTypes event)
{
    if (!statsMap.contains(event))
    {
        return "";
    }

    return statsMap[event];
}
