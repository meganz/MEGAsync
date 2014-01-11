#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <iostream>
#include <QString>
#include <QLocale>
#include <QStringList>
#include <QMutex>

#include "utils/EncryptedSettings.h"

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
    void setCredentials(QString emailHash, QString privatePw);
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
    long long lastExecutionTime();
    void setLastExecutionTime(long long time);

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
    long long getMegaFolderHandle(int num);
    QStringList getSyncNames();
    QStringList getMegaFolders();
    QStringList getLocalFolders();
    QList<long long> getMegaFolderHandles();

    void addSyncedFolder(QString localFolder, QString megaFolder, long long megaFolderHandle, QString syncName = QString());
    void removeSyncedFolder(int num);
    void removeAllFolders();

    void addRecentFile(QString fileName, long long fileHandle, QString localPath);
    QString getRecentFileName(int num);
    long long getRecentFileHandle(int num);
    QString getRecentLocalPath(int num);
    long long getRecentFileTime(int num);

    QStringList getExcludedSyncNames();
    void setExcludedSyncNames(QStringList names);

    void unlink();

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

protected:

    QMutex mutex;
    void login(QString account);
    void logout();

    void loadExcludedSyncNames();
    void readFolders();
    void writeFolders();

    void readRecentFiles();
    void writeRecentFiles();

    EncryptedSettings *settings;
    QLocale *locale;

    QStringList syncNames;
    QStringList megaFolders;
    QStringList localFolders;
    QList<long long> megaFolderHandles;

    QStringList recentFileNames;
    QList<long long> recentFileHandles;
    QStringList recentLocalPaths;
    QList<long long> recentFileTime;

    QStringList excludedSyncNames;

    static const QString currentAccountKey;
    static const QString syncsGroupKey;
    static const QString recentGroupKey;
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
    static const int NUM_RECENT_ITEMS;
};

#endif // PREFERENCES_H
