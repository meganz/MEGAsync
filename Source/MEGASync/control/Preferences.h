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

using namespace std;

class Preferences
{

private:
    static Preferences *preferences;
    Preferences();

public:
    //NOT thread-safe. Must be called before creating threads.
    static Preferences *instance();

    //Thread safe functions
    bool logged();
    bool hasEmail(QString email);
    QString email();
    void setEmail(QString email);
    QString emailHash();
    QString privatePw();
    void setSession(QString session);
    QString getSession();
    long long totalStorage();
    void setTotalStorage(long long value);
    long long usedStorage();
    void setUsedStorage(long long value);
	long long totalBandwidth();
	void setTotalBandwidth(long long value);
	long long usedBandwidth();
	void setUsedBandwidth(long long value);
    int accountType();
    void setAccountType(int value);
    bool showNotifications();
    void setShowNotifications(bool value);
    bool startOnStartup();
    void setStartOnStartup(bool value);
    QString language();
    void setLanguage(QString &value);
    bool updateAutomatically();
    void setUpdateAutomatically(bool value);
    bool canUpdate();
    int uploadLimitKB();
    void setUploadLimitKB(int value);
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
    QString getLocalFolder(int num);
    QString getMegaFolder(int num);
    mega::handle getMegaFolderHandle(int num);
    QStringList getSyncNames();
    QStringList getMegaFolders();
    QStringList getLocalFolders();
    QList<long long> getMegaFolderHandles();

    void addSyncedFolder(QString localFolder, QString megaFolder, mega::handle megaFolderHandle, QString syncName = QString());
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

    int getNumUsers();
    void enterUser(int i);
    void leaveUser();

    void unlink();

    bool isCrashed();
    void setCrashed(bool value);
    bool wasPaused();
    void setWasPaused(bool value);

    long long lastStatsRequest();
    void setLastStatsRequest(long long value);

    bool overlayIconsDisabled();
    void disableOverlayIcons(bool value);
    bool error();

    void clearAll();

    enum {
        PROXY_TYPE_NONE = 0,
        PROXY_TYPE_AUTO   = 1,
        PROXY_TYPE_CUSTOM = 2
    };

    enum {
        PROXY_PROTOCOL_HTTP = 0
    };

    enum {
        ACCOUNT_TYPE_FREE = 0,
        ACCOUNT_TYPE_PROI = 1,
        ACCOUNT_TYPE_PROII = 2,
        ACCOUNT_TYPE_PROIII = 3
    };

    static const int MAX_FILES_IN_NEW_SYNC_FOLDER;
    static const int MAX_FOLDERS_IN_NEW_SYNC_FOLDER;
    static const long long MIN_UPDATE_STATS_INTERVAL;
    static const int STATE_REFRESH_INTERVAL_MS;
    static const long long MIN_UPDATE_NOTIFICATION_INTERVAL_MS;
    static const unsigned int UPDATE_INITIAL_DELAY_SECS;
    static const unsigned int UPDATE_RETRY_INTERVAL_SECS;
    static const unsigned int UPDATE_TIMEOUT_SECS;
    static const QString UPDATE_CHECK_URL;
    static const QString CRASH_REPORT_URL;
    static const QString UPDATE_FOLDER_NAME;
    static const QString UPDATE_BACKUP_FOLDER_NAME;
    static const QString PROXY_TEST_URL;
    static const QString PROXY_TEST_SUBSTRING;
    static const char UPDATE_PUBLIC_KEY[];
    static const long long MIN_REBOOT_INTERVAL_MS;

protected:

    QMutex mutex;
    void login(QString account);
    void logout();

    void loadExcludedSyncNames();
    void readFolders();
    void writeFolders();

    EncryptedSettings *settings;
    QStringList syncNames;
    QStringList megaFolders;
    QStringList localFolders;
    QList<long long> megaFolderHandles;
    QStringList excludedSyncNames;
    bool errorFlag;

    static const QString currentAccountKey;
    static const QString syncsGroupKey;
    static const QString emailKey;
    static const QString emailHashKey;
    static const QString privatePwKey;
    static const QString totalStorageKey;
    static const QString usedStorageKey;
	static const QString totalBandwidthKey;
	static const QString usedBandwidthKey;
    static const QString accountTypeKey;
    static const QString setupWizardCompletedKey;
    static const QString showNotificationsKey;
    static const QString startOnStartupKey;
    static const QString languageKey;
    static const QString updateAutomaticallyKey;
    static const QString uploadLimitKBKey;
    static const QString proxyTypeKey;
    static const QString proxyProtocolKey;
    static const QString proxyServerKey;
    static const QString proxyPortKey;
    static const QString proxyRequiresAuthKey;
    static const QString proxyUsernameKey;
    static const QString proxyPasswordKey;
    static const QString syncNameKey;
    static const QString localFolderKey;
    static const QString megaFolderKey;
    static const QString megaFolderHandleKey;
	static const QString downloadFolderKey;
	static const QString uploadFolderKey;
	static const QString importFolderKey;
    static const QString fileNameKey;
    static const QString fileHandleKey;
    static const QString localPathKey;
    static const QString fileTimeKey;
    static const QString lastExecutionTimeKey;
    static const QString excludedSyncNamesKey;
    static const QString lastVersionKey;
    static const QString isCrashedKey;
    static const QString lastStatsRequestKey;
    static const QString wasPausedKey;
    static const QString lastUpdateTimeKey;
    static const QString lastUpdateVersionKey;
    static const QString previousCrashesKey;
    static const QString lastRebootKey;
    static const QString lastExitKey;
    static const QString disableOverlayIconsKey;
    static const QString sessionKey;

    static const bool defaultShowNotifications;
    static const bool defaultStartOnStartup;
    static const bool defaultUpdateAutomatically;
    static const int  defaultUploadLimitKB;
    static const int  defaultProxyType;
    static const QString  defaultProxyProtocol;
    static const QString  defaultProxyServer;
    static const int defaultProxyPort;
    static const bool defaultProxyRequiresAuth;
    static const QString defaultProxyUsername;
    static const QString defaultProxyPassword;	
};

#endif // PREFERENCES_H
