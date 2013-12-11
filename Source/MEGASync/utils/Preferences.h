#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <iostream>
#include <QSettings>
#include <QString>
#include <QLocale>
#include <QStringList>

Q_DECLARE_METATYPE(QList<long long>)

using namespace std;

class Preferences
{
public:
    Preferences();

    bool isTrayIconEnabled();
    void setTrayIconEnabled(bool value);
    QString email();
    void setEmail(QString email);
    QString password();
    void setPassword(QString password);
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
    bool isSetupWizardCompleted();
    void setSetupWizardCompleted(bool value);
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

	QString downloadFolder();
	void setDownloadFolder(QString value);
	long long uploadFolder();
	void setUploadFolder(long long value);
	long long importFolder();
	void setImportFolder(long long value);

    int getNumSyncedFolders();
    QString getLocalFolder(int num);
    QString getMegaFolder(int num);
    long long getMegaFolderHandle(int num);
    void addSyncedFolder(QString localFolder, QString megaFolder, long long megaFolderHandle);
    void removeSyncedFolder(int num);
    void removeAllFolders();

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

protected:

    void readFolders();
    void writeFolders();

    QSettings *settings;
    QLocale *locale;
    QStringList megaFolders;
    QStringList localFolders;
    QList<long long> megaFolderHandles;

    static const QString trayIconEnabledKey;
    static const QString emailKey;
    static const QString passwordKey;
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
    static const QString localFoldersKey;
    static const QString megaFoldersKey;
    static const QString megaFolderHandlesKey;
	static const QString downloadFolderKey;
	static const QString uploadFolderKey;
	static const QString importFolderKey;

    static const bool defaultSetupWizardCompleted;
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
