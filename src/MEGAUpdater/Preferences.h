#ifndef PREFERENCES_H
#define PREFERENCES_H

#if defined(__APPLE__)
#include "../MEGASync/platform/macx/MacUpdateRouting.h"
#endif

const char CLIENT_KEY[] = "FhMgXbqb";
const char USER_AGENT[] = "MEGA/MEGAUpdaterTask";

#ifdef _WIN32
    #ifdef _WIN64
        const char UPDATE_CHECK_URL[] = "http://g.static.mega.co.nz/eupd/wsync64/v.txt";
    #else
        const char UPDATE_CHECK_URL[]  = "http://g.static.mega.co.nz/eupd/wsync/v.txt";
    #endif
#else
const char APP_DIR_BUNDLE[] = "/Applications/MEGAsync.app/";
#endif

        const char UPDATE_PUBLIC_KEY[] =
            "EACTzXPE8fdMhm6LizLe1FxV2DncybVh2cXpW3momTb8tpzRNT833r1RfySz5uHe8gdoXN1W0eM5Bk8X-"
            "LefygYYDS9RyXrRZ8qXrr9ITJ4r8ATnFIEThO5vqaCpGWTVi5pOPI5FUTJuhghVKTyAels2SpYT5CmfSQIkMKv"
            "7YVldaV7A-kY060GfrNg4--ETyIzhvaSZ_jyw-gmzYl_"
            "dwfT9kSzrrWy1vQG8JPNjKVPC4MCTZJx9SNvp1fVi77hhgT-"
            "Mc5PLcDIfjustlJkDBHtmGEjyaDnaWQf49rGq94q23mLc56MSjKpjOR1TtpsCY31d1Oy2fEXFgghM0R-"
            "1UkKswVuWhEEd8nO2PimJOl4u9ZJ2PWtJL1Ro0Hlw9OemJ12klIAxtGV-"
            "61Z60XoErbqThwWT5Uu3D2gjK9e6rL9dufSoqjC7UA2C0h7KNtfUcUHw0UWzahlR8XBNFXaLWx9Z8fRtA_"
            "a4seZcr0AhIA7JdQG5i8tOZo966KcFnkU77pfQTSprnJhCfEmYbWm9EZA122LJBWq2UrSQQN3pKc9goNaaNxy5"
            "PYU1yXyiAfMVsBDmDonhRWQh2XhdV-FWJ3rOGMe25zOwV4z1XkNBuW4T1JF2FgqGR6_"
            "q74B2ccFC8vrNGvlTEcs3MSxTI_EKLXQvBYy7hxG8EPUkrMVCaWzzTQAFEQ";
        const char UPDATE_FILENAME[] = "v.txt";
        const char UPDATE_FOLDER_NAME[] = "eupdate";
        const char BACKUP_FOLDER_NAME[] = "ebackup";
        const char VERSION_FILE_NAME[] = "megasync.version";
        // Manifest-relative path of the application binary. Used as the
        // installation anchor: the obsolete-file sweep only trusts appFolder
        // when this file was already present before the run started, and only
        // trusts a manifest that lists it.
#ifdef _WIN32
        const char APP_BINARY_RELATIVE_PATH[] = "MEGAsync.exe";
#else
        const char APP_BINARY_RELATIVE_PATH[] = "Contents/MacOS/MEGAsync";
#endif
#ifndef _WIN32
        // Bundle-relative location of the symlink recipe delivered by the update
        // manifest. Used by the every-run symlink restoration in checkForUpdates();
        // performUpdate() additionally resolves it from the manifest itself, which
        // covers a future relocation.
        const char SYMLINKS_FILE_RELATIVE_PATH[] = "Contents/Resources/mega.links";
#endif

#endif // PREFERENCES_H
