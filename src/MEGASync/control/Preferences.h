#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <iostream>
#include <QString>
#include <QLocale>
#include <QStringList>
#include <QMutex>

#include "control/EncryptedSettings.h"
#include "megaapi.h"

Q_DECLARE_METATYPE(QList<long long>)

class Preferences : public QObject
{
    Q_OBJECT

signals:
    void stateChanged();
    void updated(int lastVersion);

private:
    static Preferences *preferences;
    Preferences();

public:
    //NOT thread-safe. Must be called before creating threads.
    static Preferences *instance();

    void initialize(QString dataPath);

    //Thread safe functions
    bool logged();
    bool hasEmail(QString email);
    QString email();
    void setEmail(QString email);
    QString firstName();
    void setFirstName(QString firstName);
    QString lastName();
    void setLastName(QString lastName);
    QString emailHash();
    QString privatePw();
    void setSession(QString session);
    QString getSession();
    long long totalStorage();
    void setTotalStorage(long long value);
    long long usedStorage();
    void setUsedStorage(long long value);

    long long cloudDriveStorage();
    void setCloudDriveStorage(long long value);
    long long inboxStorage();
    void setInboxStorage(long long value);
    long long rubbishStorage();
    void setRubbishStorage(long long value);
    long long inShareStorage();
    void setInShareStorage(long long value);

    long long cloudDriveFiles();
    void setCloudDriveFiles(long long value);
    long long inboxFiles();
    void setInboxFiles(long long value);
    long long rubbishFiles();
    void setRubbishFiles(long long value);
    long long inShareFiles();
    void setInShareFiles(long long value);

    long long cloudDriveFolders();
    void setCloudDriveFolders(long long value);
    long long inboxFolders();
    void setInboxFolders(long long value);
    long long rubbishFolders();
    void setRubbishFolders(long long value);
    long long inShareFolders();
    void setInShareFolders(long long value);

    long long totalBandwidth();
    void setTotalBandwidth(long long value);
    bool isTemporalBandwidthValid();
    long long getMsDiffTimeWithSDK();
    void setDsDiffTimeWithSDK(long long diffTime);
    void setTemporalBandwidthValid(bool value);
    long long temporalBandwidth();
    void setTemporalBandwidth(long long value);
    int temporalBandwidthInterval();
    void setTemporalBandwidthInterval(int value);
    long long usedBandwidth();
    void setUsedBandwidth(long long value);
    int accountType();
    void setAccountType(int value);
    bool showNotifications();
    void setShowNotifications(bool value);
    bool startOnStartup();
    void setStartOnStartup(bool value);
    bool usingHttpsOnly();
    void setUseHttpsOnly(bool value);
    bool SSLcertificateException();
    void setSSLcertificateException(bool value);
    QString language();
    void setLanguage(QString &value);
    bool updateAutomatically();
    void setUpdateAutomatically(bool value);
    bool hasDefaultUploadFolder();
    bool hasDefaultDownloadFolder();
    bool hasDefaultImportFolder();
    void setHasDefaultUploadFolder(bool value);
    void setHasDefaultDownloadFolder(bool value);
    void setHasDefaultImportFolder(bool value);
    bool canUpdate(QString filePath);
    int uploadLimitKB();
    int downloadLimitKB();
    void setUploadLimitKB(int value);
    void setDownloadLimitKB(int value);
    int parallelUploadConnections();
    int parallelDownloadConnections();
    void setParallelUploadConnections(int value);
    void setParallelDownloadConnections(int value);
    long long upperSizeLimitValue();
    void setUpperSizeLimitValue(long long value);
    long long lowerSizeLimitValue();
    void setLowerSizeLimitValue(long long value);
    bool upperSizeLimit();
    void setUpperSizeLimit(bool value);
    bool lowerSizeLimit();
    void setLowerSizeLimit(bool value);
    int upperSizeLimitUnit();
    void setUpperSizeLimitUnit(int value);
    int lowerSizeLimitUnit();
    void setLowerSizeLimitUnit(int value);
    int folderPermissionsValue();
    void setFolderPermissionsValue(int permissions);
    int filePermissionsValue();
    void setFilePermissionsValue(int permissions);
    int proxyType();
    void setProxyType(int value);
    int proxyProtocol();
    void setProxyProtocol(int value);
    QString proxyServer();
    void setProxyServer(const QString &value);
    int proxyPort();
    void setProxyPort(int value);
    bool proxyRequiresAuth();
    void setProxyRequiresAuth(bool value);
    QString getProxyUsername();
    void setProxyUsername(const QString &value);
    QString getProxyPassword();
    void setProxyPassword(const QString &value);
    QString proxyHostAndPort();
    long long lastExecutionTime();
    void setLastExecutionTime(long long time);
    long long installationTime();
    void setInstallationTime(long long time);
    long long accountCreationTime();
    void setAccountCreationTime(long long time);
    long long hasLoggedIn();
    void setHasLoggedIn(long long time);
    bool isFirstStartDone();
    void setFirstStartDone(bool value = true);
    bool isFirstSyncDone();
    void setFirstSyncDone(bool value = true);
    bool isFirstFileSynced();
    void setFirstFileSynced(bool value = true);
    bool isFirstWebDownloadDone();
    void setFirstWebDownloadDone(bool value = true);
    bool isFatWarningShown();
    void setFatWarningShown(bool value = true);
    QString lastCustomStreamingApp();
    void setLastCustomStreamingApp(const QString &value);
    long long getMaxMemoryUsage();
    void setMaxMemoryUsage(long long value);
    long long getMaxMemoryReportTime();
    void setMaxMemoryReportTime(long long timestamp);

    int transferDownloadMethod();
    void setTransferDownloadMethod(int value);
    int transferUploadMethod();
    void setTransferUploadMethod(int value);

    long long lastUpdateTime();
    void setLastUpdateTime(long long time);
    int lastUpdateVersion();
    void setLastUpdateVersion(int version);

    QString downloadFolder();
    void setDownloadFolder(QString value);
    long long uploadFolder();
    void setUploadFolder(long long value);
    long long importFolder();
    void setImportFolder(long long value);

    int getNumSyncedFolders();
    QString getSyncName(int num);
    QString getSyncID(int num);
    QString getLocalFolder(int num);
    QString getMegaFolder(int num);
    long long getLocalFingerprint(int num);
    void setLocalFingerprint(int num, long long fingerprint);
    mega::MegaHandle getMegaFolderHandle(int num);
    bool isFolderActive(int num);
    bool isTemporaryInactiveFolder(int num);
    void setSyncState(int num, bool enabled, bool temporaryDisabled = false);

    bool isOneTimeActionDone(int action);
    void setOneTimeActionDone(int action, bool done);

    QStringList getSyncNames();
    QStringList getSyncIDs();
    QStringList getMegaFolders();
    QStringList getLocalFolders();
    QList<long long> getMegaFolderHandles();

    void addSyncedFolder(QString localFolder, QString megaFolder, mega::MegaHandle megaFolderHandle, QString syncName = QString(), bool active = true);
    void setMegaFolderHandle(int num, mega::MegaHandle handle);
    void removeSyncedFolder(int num);
    void removeAllFolders();

    QStringList getExcludedSyncNames();
    void setExcludedSyncNames(QStringList names);
    QStringList getPreviousCrashes();
    void setPreviousCrashes(QStringList crashes);
    long long getLastReboot();
    void setLastReboot(long long value);
    long long getLastExit();
    void setLastExit(long long value);

    QString getHttpsKey();
    void setHttpsKey(QString key);
    QString getHttpsCert();
    void setHttpsCert(QString cert);
    QString getHttpsCertIntermediate();
    void setHttpsCertIntermediate(QString intermediate);
    long long getHttpsCertExpiration();
    void setHttpsCertExpiration(long long expiration);

    int getNumUsers();
    void enterUser(int i);
    void leaveUser();

    void unlink();

    bool isCrashed();
    void setCrashed(bool value);
    bool getGlobalPaused();
    void setGlobalPaused(bool value);
    bool getUploadsPaused();
    void setUploadsPaused(bool value);
    bool getDownloadsPaused();
    void setDownloadsPaused(bool value);

    long long lastStatsRequest();
    void setLastStatsRequest(long long value);

    bool overlayIconsDisabled();
    void disableOverlayIcons(bool value);
    bool error();

    QString getDataPath();
    void clearTemporalBandwidth();
    void clearAll();
    void sync();

    enum {
        PROXY_TYPE_NONE = 0,
        PROXY_TYPE_AUTO   = 1,
        PROXY_TYPE_CUSTOM = 2
    };

    enum {
        PROXY_PROTOCOL_HTTP = 0,
        PROXY_PROTOCOL_SOCKS5H = 1
    };

    enum {
        BYTE_UNIT = 0,
        KILO_BYTE_UNIT = 1,
        MEGA_BYTE_UNIT = 2,
        GIGA_BYTE_UNIT = 3
    };

    enum {
        ACCOUNT_TYPE_FREE = 0,
        ACCOUNT_TYPE_PROI = 1,
        ACCOUNT_TYPE_PROII = 2,
        ACCOUNT_TYPE_PROIII = 3,
        ACCOUNT_TYPE_LITE = 4
    };

    enum {
        ONE_TIME_ACTION_DEPRECATED_OPERATING_SYSTEM = 0,
        ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE = 1
    };

    static const int MAX_FILES_IN_NEW_SYNC_FOLDER;
    static const int MAX_FOLDERS_IN_NEW_SYNC_FOLDER;
    static const long long MIN_UPDATE_STATS_INTERVAL;
    static const long long MIN_UPDATE_STATS_INTERVAL_OVERQUOTA;
    static const int STATE_REFRESH_INTERVAL_MS;
    static const int FINISHED_TRANSFER_REFRESH_INTERVAL_MS;
    static const long long MIN_UPDATE_NOTIFICATION_INTERVAL_MS;
    static const unsigned int UPDATE_INITIAL_DELAY_SECS;
    static const unsigned int UPDATE_RETRY_INTERVAL_SECS;
    static const unsigned int UPDATE_TIMEOUT_SECS;
    static const unsigned int MAX_LOGIN_TIME_MS;
    static const QString UPDATE_CHECK_URL;
    static const QString CRASH_REPORT_URL;
    static const QString UPDATE_FOLDER_NAME;
    static const QString UPDATE_BACKUP_FOLDER_NAME;
    static const QString PROXY_TEST_URL;
    static const QString PROXY_TEST_SUBSTRING;
    static const unsigned int PROXY_TEST_TIMEOUT_MS;
    static const QString LOCAL_HTTPS_TEST_URL;
    static const QString LOCAL_HTTPS_TEST_POST_DATA;
    static const QString LOCAL_HTTPS_TEST_SUBSTRING;
    static const unsigned int LOCAL_HTTPS_TEST_TIMEOUT_MS;
    static const long long LOCAL_HTTPS_CERT_MAX_EXPIRATION_SECS;
    static const unsigned int MAX_IDLE_TIME_MS;
    static const char UPDATE_PUBLIC_KEY[];
    static const long long MIN_REBOOT_INTERVAL_MS;
    static const long long MIN_EXTERNAL_NODES_WARNING_MS;
    static const char CLIENT_KEY[];
    static const char USER_AGENT[];
    static const int VERSION_CODE;
    static const int BUILD_ID;
    static const QString VERSION_STRING;
    static const QString SDK_ID;
    static const QString CHANGELOG;
    static const QString TRANSLATION_FOLDER;
    static const QString TRANSLATION_PREFIX;
    static const qint16 HTTPS_PORT;

    static QStringList HTTPS_ALLOWED_ORIGINS;
    static bool HTTPS_ORIGIN_CHECK_ENABLED;
    static const unsigned int MAX_COMPLETED_ITEMS;
    static const QString FINDER_EXT_BUNDLE_ID;

protected:
    QMutex mutex;
    void login(QString account);
    void logout();

    void loadExcludedSyncNames();
    void readFolders();
    void writeFolders();

    EncryptedSettings *settings;
    QStringList syncNames;
    QStringList syncIDs;
    QStringList megaFolders;
    QStringList localFolders;
    QList<long long> megaFolderHandles;
    QList<long long> localFingerprints;
    QList<bool> activeFolders;
    QList<bool> temporaryInactiveFolders;
    QStringList excludedSyncNames;
    bool errorFlag;
    long long tempBandwidth;
    int tempBandwidthInterval;
    bool isTempBandwidthValid;
    QString dataPath;
    long long diffTimeWithSDK;

    static const QString currentAccountKey;
    static const QString syncsGroupKey;
    static const QString emailKey;
    static const QString firstNameKey;
    static const QString lastNameKey;
    static const QString emailHashKey;
    static const QString privatePwKey;
    static const QString totalStorageKey;
    static const QString usedStorageKey;
    static const QString cloudDriveStorageKey;
    static const QString inboxStorageKey;
    static const QString rubbishStorageKey;
    static const QString inShareStorageKey;
    static const QString cloudDriveFilesKey;
    static const QString inboxFilesKey;
    static const QString rubbishFilesKey;
    static const QString inShareFilesKey;
    static const QString cloudDriveFoldersKey;
    static const QString inboxFoldersKey;
    static const QString rubbishFoldersKey;
    static const QString inShareFoldersKey;
    static const QString totalBandwidthKey;
    static const QString usedBandwidthKey;
    static const QString accountTypeKey;
    static const QString setupWizardCompletedKey;
    static const QString showNotificationsKey;
    static const QString startOnStartupKey;
    static const QString languageKey;
    static const QString updateAutomaticallyKey;
    static const QString uploadLimitKBKey;
    static const QString downloadLimitKBKey;
    static const QString parallelUploadConnectionsKey;
    static const QString parallelDownloadConnectionsKey;
    static const QString upperSizeLimitKey;
    static const QString lowerSizeLimitKey;
    static const QString upperSizeLimitValueKey;
    static const QString lowerSizeLimitValueKey;
    static const QString upperSizeLimitUnitKey;
    static const QString lowerSizeLimitUnitKey;
    static const QString folderPermissionsKey;
    static const QString filePermissionsKey;
    static const QString proxyTypeKey;
    static const QString proxyProtocolKey;
    static const QString proxyServerKey;
    static const QString proxyPortKey;
    static const QString proxyRequiresAuthKey;
    static const QString proxyUsernameKey;
    static const QString proxyPasswordKey;
    static const QString syncNameKey;
    static const QString syncIdKey;
    static const QString localFolderKey;
    static const QString megaFolderKey;
    static const QString megaFolderHandleKey;
    static const QString folderActiveKey;
    static const QString temporaryInactiveKey;
    static const QString downloadFolderKey;
    static const QString uploadFolderKey;
    static const QString hasDefaultUploadFolderKey;
    static const QString hasDefaultDownloadFolderKey;
    static const QString hasDefaultImportFolderKey;
    static const QString importFolderKey;
    static const QString fileNameKey;
    static const QString fileHandleKey;
    static const QString localPathKey;
    static const QString localFingerprintKey;
    static const QString fileTimeKey;
    static const QString lastExecutionTimeKey;
    static const QString excludedSyncNamesKey;
    static const QString lastVersionKey;
    static const QString isCrashedKey;
    static const QString lastStatsRequestKey;
    static const QString wasPausedKey;
    static const QString wasUploadsPausedKey;
    static const QString wasDownloadsPausedKey;
    static const QString lastUpdateTimeKey;
    static const QString lastUpdateVersionKey;
    static const QString previousCrashesKey;
    static const QString lastRebootKey;
    static const QString lastExitKey;
    static const QString disableOverlayIconsKey;
    static const QString sessionKey;
    static const QString firstStartDoneKey;
    static const QString firstSyncDoneKey;
    static const QString firstFileSyncedKey;
    static const QString firstWebDownloadKey;
    static const QString fatWarningShownKey;
    static const QString installationTimeKey;
    static const QString accountCreationTimeKey;
    static const QString hasLoggedInKey;
    static const QString transferDownloadMethodKey;
    static const QString transferUploadMethodKey;
    static const QString lastCustomStreamingAppKey;
    static const QString useHttpsOnlyKey;
    static const QString SSLcertificateExceptionKey;
    static const QString maxMemoryUsageKey;
    static const QString maxMemoryReportTimeKey;
    static const QString oneTimeActionDoneKey;
    static const QString httpsKeyKey;
    static const QString httpsCertKey;
    static const QString httpsCertIntermediateKey;
    static const QString httpsCertExpirationKey;

    static const bool defaultShowNotifications;
    static const bool defaultStartOnStartup;
    static const bool defaultUpdateAutomatically;
    static const int  defaultUploadLimitKB;
    static const int  defaultDownloadLimitKB;
    static const int  defaultParallelUploadConnections;
    static const int  defaultParallelDownloadConnections;
    static const int  defaultProxyType;
    static const int  defaultProxyProtocol;
    static const QString  defaultProxyServer;
    static const int defaultProxyPort;
    static const bool defaultProxyRequiresAuth;
    static const QString defaultProxyUsername;
    static const QString defaultProxyPassword;
    static const bool defaultUpperSizeLimit;
    static const bool defaultLowerSizeLimit;
    static const long long defaultUpperSizeLimitValue;
    static const long long defaultLowerSizeLimitValue;
    static const int defaultUpperSizeLimitUnit;
    static const int defaultLowerSizeLimitUnit;
    static const int defaultTransferDownloadMethod;
    static const int defaultTransferUploadMethod;
    static const int defaultFolderPermissions;
    static const int defaultFilePermissions;
    static const bool defaultUseHttpsOnly;
    static const bool defaultSSLcertificateException;
    static const QString defaultHttpsKey;
    static const QString defaultHttpsCert;
    static const QString defaultHttpsCertIntermediate;
    static const long long defaultHttpsCertExpiration;
};

#endif // PREFERENCES_H
