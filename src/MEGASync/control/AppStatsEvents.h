#ifndef APPSTATSEVENTS_H
#define APPSTATSEVENTS_H

// Event IDs sent to servers for statistics purpose.
enum AppStatsEvents
{
    EVENT_1ST_START                                 = 99500,
    EVENT_1ST_SYNC                                  = 99501,
    EVENT_1ST_SYNCED_FILE                           = 99502,
    EVENT_1ST_WEBCLIENT_DL                          = 99503,
    EVENT_INSTALL_STATS                             = 99504,
    EVENT_ACC_CREATION_START                        = 99505,
//  EVENT_                                          = 99506,
//  EVENT_                                          = 99507,
    EVENT_PRO_REDIRECT                              = 99508,
    EVENT_MEM_USAGE                                 = 99509,
    EVENT_UPDATE                                    = 99510,
    EVENT_UPDATE_OK                                 = 99511,
    EVENT_DUP_FINISHED_TRSF                         = 99512,
    EVENT_DUP_ACTIVE_TRSF_DURING_INIT               = 99513,
    EVENT_DUP_ACTIVE_TRSF_DURING_INSERT             = 99514,
//  EVENT_                                          = 99515,
//  EVENT_                                          = 99516,
    EVENT_LOCAL_SSL_CERT_RENEWED                    = 99517,
    EVENT_OVER_STORAGE_DIAL                         = 99518,
    EVENT_OVER_STORAGE_NOTIF                        = 99519,
    EVENT_OVER_STORAGE_MSG                          = 99520,
    EVENT_ALMOST_OVER_STORAGE_MSG                   = 99521,
    EVENT_ALMOST_OVER_STORAGE_NOTIF                 = 99522,
    EVENT_MAIN_DIAL_WHILE_OVER_QUOTA                = 99523,
    EVENT_MAIN_DIAL_WHILE_ALMOST_OVER_QUOTA         = 99524,
    EVENT_RED_LIGHT_USED_STORAGE_MISMATCH           = 99525,
    EVENT_TRSF_OVER_QUOTA_DIAL                      = 99526,
    EVENT_TRSF_OVER_QUOTA_NOTIF                     = 99527,
    EVENT_TRSF_OVER_QUOTA_MSG                       = 99528,
    EVENT_TRSF_ALMOST_OVER_QUOTA_MSG                = 99529,
    EVENT_PAYWALL_NOTIF                             = 99530,
    EVENT_SYNC_ADD_FAIL_API_EACCESS                 = 99531,
    EVENT_TRSF_ALMOST_OVERQUOTA_NOTIF               = 99532,
    EVENT_1ST_BACKUP                                = 99533,
    EVENT_1ST_BACKED_UP_FILE                        = 99534,
    //STALLED ISSUES EVENTS
    EVENT_SI_NAMECONFLICT_SOLVED_MANUALLY           = 99535,
    EVENT_SI_NAMECONFLICT_SOLVED_AUTOMATICALLY      = 99536,
    EVENT_SI_NAMECONFLICT_SOLVED_SEMI_AUTOMATICALLY = 99537,
    EVENT_SI_LOCALREMOTE_SOLVED_MANUALLY            = 99538,
    EVENT_SI_LOCALREMOTE_SOLVED_AUTOMATICALLY       = 99539,
    EVENT_SI_LOCALREMOTE_SOLVED_SEMI_AUTOMATICALLY  = 99540,
    EVENT_SI_IGNORE_SOLVED_MANUALLY                 = 99541,
    EVENT_SI_STALLED_ISSUE_RESERVED                 = 99565,
};

#endif // APPSTATSEVENTS_H
