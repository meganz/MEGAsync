#include "Preferences.h"
#include "platform/Platform.h"

#include <QDesktopServices>
#include <assert.h>

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

const int Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER     = 80000;
const int Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER   = 15000;
const int Preferences::REBOOT_DELAY_MS                  = 10000;
const int Preferences::STATE_REFRESH_INTERVAL_MS        = 10000;
const long long Preferences::MIN_UPDATE_STATS_INTERVAL  = 300000;
const long long Preferences::MIN_UPDATE_NOTIFICATION_INTERVAL_MS    = 172800000;
const long long Preferences::MIN_REBOOT_INTERVAL_MS                 = 300000;

const unsigned int Preferences::UPDATE_INITIAL_DELAY_SECS           = 60;
const unsigned int Preferences::UPDATE_RETRY_INTERVAL_SECS          = 7200;
const QString Preferences::UPDATE_CHECK_URL                 = QString::fromUtf8("http://g.static.mega.co.nz/upd/wsync/v.txt");
const QString Preferences::CRASH_REPORT_URL                 = QString::fromUtf8("http://g.api.mega.co.nz/hb?crashdump");
const QString Preferences::UPDATE_FOLDER_NAME               = QString::fromAscii("update");
const QString Preferences::UPDATE_BACKUP_FOLDER_NAME        = QString::fromAscii("backup");
const char Preferences::UPDATE_PUBLIC_KEY[] = "EACTzXPE8fdMhm6LizLe1FxV2DncybVh2cXpW3momTb8tpzRNT833r1RfySz5uHe8gdoXN1W0eM5Bk8X-LefygYYDS9RyXrRZ8qXrr9ITJ4r8ATnFIEThO5vqaCpGWTVi5pOPI5FUTJuhghVKTyAels2SpYT5CmfSQIkMKv7YVldaV7A-kY060GfrNg4--ETyIzhvaSZ_jyw-gmzYl_dwfT9kSzrrWy1vQG8JPNjKVPC4MCTZJx9SNvp1fVi77hhgT-Mc5PLcDIfjustlJkDBHtmGEjyaDnaWQf49rGq94q23mLc56MSjKpjOR1TtpsCY31d1Oy2fEXFgghM0R-1UkKswVuWhEEd8nO2PimJOl4u9ZJ2PWtJL1Ro0Hlw9OemJ12klIAxtGV-61Z60XoErbqThwWT5Uu3D2gjK9e6rL9dufSoqjC7UA2C0h7KNtfUcUHw0UWzahlR8XBNFXaLWx9Z8fRtA_a4seZcr0AhIA7JdQG5i8tOZo966KcFnkU77pfQTSprnJhCfEmYbWm9EZA122LJBWq2UrSQQN3pKc9goNaaNxy5PYU1yXyiAfMVsBDmDonhRWQh2XhdV-FWJ3rOGMe25zOwV4z1XkNBuW4T1JF2FgqGR6_q74B2ccFC8vrNGvlTEcs3MSxTI_EKLXQvBYy7hxG8EPUkrMVCaWzzTQAFEQ";

const QString Preferences::syncsGroupKey            = QString::fromAscii("Syncs");
const QString Preferences::recentGroupKey           = QString::fromAscii("Recent");
const QString Preferences::currentAccountKey        = QString::fromAscii("currentAccount");
const QString Preferences::emailKey                 = QString::fromAscii("email");
const QString Preferences::emailHashKey             = QString::fromAscii("emailHash");
const QString Preferences::privatePwKey             = QString::fromAscii("privatePw");
const QString Preferences::totalStorageKey          = QString::fromAscii("totalStorage");
const QString Preferences::usedStorageKey           = QString::fromAscii("usedStorage");
const QString Preferences::totalBandwidthKey		= QString::fromAscii("totalBandwidth");
const QString Preferences::usedBandwidthKey			= QString::fromAscii("usedBandwidth");
const QString Preferences::accountTypeKey           = QString::fromAscii("accountType");
const QString Preferences::showNotificationsKey     = QString::fromAscii("showNotifications");
const QString Preferences::startOnStartupKey        = QString::fromAscii("startOnStartup");
const QString Preferences::languageKey              = QString::fromAscii("language");
const QString Preferences::updateAutomaticallyKey   = QString::fromAscii("updateAutomatically");
const QString Preferences::uploadLimitKBKey         = QString::fromAscii("uploadLimitKB");
const QString Preferences::proxyTypeKey             = QString::fromAscii("proxyType");
const QString Preferences::proxyProtocolKey         = QString::fromAscii("proxyProtocol");
const QString Preferences::proxyServerKey           = QString::fromAscii("proxyServer");
const QString Preferences::proxyPortKey             = QString::fromAscii("proxyPort");
const QString Preferences::proxyRequiresAuthKey     = QString::fromAscii("proxyRequiresAuth");
const QString Preferences::proxyUsernameKey         = QString::fromAscii("proxyUsername");
const QString Preferences::proxyPasswordKey         = QString::fromAscii("proxyPassword");
const QString Preferences::syncNameKey              = QString::fromAscii("syncName");
const QString Preferences::localFolderKey           = QString::fromAscii("localFolder");
const QString Preferences::megaFolderKey            = QString::fromAscii("megaFolder");
const QString Preferences::megaFolderHandleKey      = QString::fromAscii("megaFolderHandle");
const QString Preferences::downloadFolderKey		= QString::fromAscii("downloadFolder");
const QString Preferences::uploadFolderKey			= QString::fromAscii("uploadFolder");
const QString Preferences::importFolderKey			= QString::fromAscii("importFolder");
const QString Preferences::fileNameKey              = QString::fromAscii("fileName");
const QString Preferences::fileHandleKey            = QString::fromAscii("fileHandle");
const QString Preferences::localPathKey             = QString::fromAscii("localPath");
const QString Preferences::fileTimeKey              = QString::fromAscii("fileTime");
const QString Preferences::isCrashedKey             = QString::fromAscii("isCrashed");
const QString Preferences::wasPausedKey             = QString::fromAscii("wasPaused");
const QString Preferences::lastExecutionTimeKey     = QString::fromAscii("lastExecutionTime");
const QString Preferences::excludedSyncNamesKey     = QString::fromAscii("excludedSyncNames");
const QString Preferences::lastVersionKey           = QString::fromAscii("lastVersion");
const QString Preferences::lastStatsRequestKey      = QString::fromAscii("lastStatsRequest");
const QString Preferences::lastUpdateTimeKey        = QString::fromAscii("lastUpdateTime");
const QString Preferences::lastUpdateVersionKey     = QString::fromAscii("lastUpdateVersion");
const QString Preferences::previousCrashesKey       = QString::fromAscii("previousCrashes");
const QString Preferences::lastRebootKey            = QString::fromAscii("lastReboot");
const QString Preferences::lastExitKey            = QString::fromAscii("lastExit");

const bool Preferences::defaultShowNotifications    = false;
const bool Preferences::defaultStartOnStartup       = true;
const bool Preferences::defaultUpdateAutomatically  = true;
const int  Preferences::defaultUploadLimitKB        = -1;
const int  Preferences::defaultProxyType            = PROXY_TYPE_AUTO;
const QString  Preferences::defaultProxyProtocol    = QString::fromAscii("HTTP");
const QString  Preferences::defaultProxyServer      = QString::fromAscii("127.0.0.1");
const int Preferences::defaultProxyPort             = 8080;
const bool Preferences::defaultProxyRequiresAuth    = false;
const QString Preferences::defaultProxyUsername     = QString::fromAscii("");
const QString Preferences::defaultProxyPassword     = QString::fromAscii("");
const int Preferences::NUM_RECENT_ITEMS             = 3;

Preferences *Preferences::preferences = NULL;

Preferences *Preferences::instance()
{
    if(!preferences) preferences = new Preferences();
    return Preferences::preferences;
}

Preferences::Preferences() : mutex(QMutex::Recursive)
{
#if QT_VERSION < 0x050000
    QString dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString newPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0];
#endif

    QDir dir(dataPath);
    dir.mkpath(QString::fromAscii("."));
    QString settingsFile = QDir::toNativeSeparators(dataPath + QString::fromAscii("/MEGAsync.cfg"));

    settings = new EncryptedSettings(settingsFile);

    QString currentAccount = settings->value(currentAccountKey).toString();
    if(currentAccount.size()) login(currentAccount);
}

QString Preferences::email()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(emailKey).toString();
    mutex.unlock();
    return value;
}

void Preferences::setEmail(QString email)
{
    mutex.lock();
    login(email);
    settings->setValue(emailKey, email);
    settings->sync();
    mutex.unlock();
}

QString Preferences::emailHash()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(emailHashKey).toString();
    mutex.unlock();
    return value;
}

QString Preferences::privatePw()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(privatePwKey).toString();
    mutex.unlock();
    return value;
}

void Preferences::setCredentials(QString emailHash, QString privatePw)
{
    mutex.lock();
    assert(logged());
    settings->setValue(emailHashKey, emailHash);
    settings->setValue(privatePwKey, privatePw);
    settings->sync();
    mutex.unlock();
}

long long Preferences::totalStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(totalStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setTotalStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(totalStorageKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::usedStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(usedStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setUsedStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(usedStorageKey, value);
	settings->sync();
    mutex.unlock();
}

long long Preferences::totalBandwidth()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(totalBandwidthKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setTotalBandwidth(long long value)
{
    mutex.lock();
    assert(logged());
	settings->setValue(totalBandwidthKey, value);
	settings->sync();
    mutex.unlock();
}

long long Preferences::usedBandwidth()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(usedBandwidthKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setUsedBandwidth(long long value)
{
    mutex.lock();
    assert(logged());
	settings->setValue(usedBandwidthKey, value);
	settings->sync();
    mutex.unlock();
}

int Preferences::accountType()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(accountTypeKey).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setAccountType(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(accountTypeKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::showNotifications()
{
    mutex.lock();
    assert(logged());
    bool value = settings->value(showNotificationsKey, defaultShowNotifications).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setShowNotifications(bool value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(showNotificationsKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::startOnStartup()
{
    mutex.lock();
    bool value = settings->value(startOnStartupKey, defaultStartOnStartup).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setStartOnStartup(bool value)
{
    mutex.lock();
    settings->setValue(startOnStartupKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::language()
{
    mutex.lock();
    QString value = settings->value(languageKey, QLocale::system().name()).toString();
    mutex.unlock();
    return value;
}

void Preferences::setLanguage(QString &value)
{
    mutex.lock();
    settings->setValue(languageKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::updateAutomatically()
{
    mutex.lock();
    bool value = settings->value(updateAutomaticallyKey, defaultUpdateAutomatically).toBool();

#ifdef WIN32
    qt_ntfs_permission_lookup++; // turn checking on
#endif
    if(value && !QFileInfo(MegaApplication::applicationFilePath()).isWritable())
    {
        this->setUpdateAutomatically(false);
        value = false;
    }
#ifdef WIN32
    qt_ntfs_permission_lookup--; // turn it off again
#endif
    mutex.unlock();
    return value;
}

void Preferences::setUpdateAutomatically(bool value)
{
    mutex.lock();
    settings->setValue(updateAutomaticallyKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::canUpdate()
{
    mutex.lock();

    bool value = true;

#ifdef WIN32
    qt_ntfs_permission_lookup++; // turn checking on
#endif
    if(!QFileInfo(MegaApplication::applicationFilePath()).isWritable())
        value = false;
#ifdef WIN32
    qt_ntfs_permission_lookup--; // turn it off again
#endif

    mutex.unlock();

    return value;
}

int Preferences::uploadLimitKB()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(uploadLimitKBKey, defaultUploadLimitKB).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setUploadLimitKB(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(uploadLimitKBKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyType()
{
    mutex.lock();
    int value = settings->value(proxyTypeKey, defaultProxyType).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setProxyType(int value)
{
    mutex.lock();
    settings->setValue(proxyTypeKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyProtocol()
{
    mutex.lock();
    int value = settings->value(proxyProtocolKey, defaultProxyProtocol).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setProxyProtocol(int value)
{
    mutex.lock();
    settings->setValue(proxyProtocolKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::proxyServer()
{
    mutex.lock();
    QString value = settings->value(proxyServerKey, defaultProxyServer).toString();
    mutex.unlock();
    return value;
}

void Preferences::setProxyServer(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyServerKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyPort()
{
    mutex.lock();
    int value = settings->value(proxyPortKey, defaultProxyPort).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setProxyPort(int value)
{
    mutex.lock();
    settings->setValue(proxyPortKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::proxyRequiresAuth()
{
    mutex.lock();
    bool value = settings->value(proxyRequiresAuthKey, defaultProxyRequiresAuth).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setProxyRequiresAuth(bool value)
{
    mutex.lock();
    settings->setValue(proxyRequiresAuthKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::getProxyUsername()
{
    mutex.lock();
    QString value = settings->value(proxyUsernameKey, defaultProxyUsername).toString();
    mutex.unlock();
    return value;
}

void Preferences::setProxyUsername(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyUsernameKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::getProxyPassword()
{
    mutex.lock();
    QString value = settings->value(proxyPasswordKey, defaultProxyPassword).toString();
    mutex.unlock();
    return value;
}

void Preferences::setProxyPassword(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyPasswordKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::proxyHostAndPort()
{
    mutex.lock();
    QString proxy;
    proxy.append(proxyServer());
    proxy.append(QChar::fromAscii(':'));
    proxy.append(QString::number(proxyPort()));
    mutex.unlock();

    return proxy;
}

long long Preferences::lastExecutionTime()
{
    mutex.lock();
    long long value = settings->value(lastExecutionTimeKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setLastExecutionTime(qint64 time)
{
    mutex.lock();
    settings->setValue(lastExecutionTimeKey, time);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lastUpdateTime()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(lastUpdateTimeKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setLastUpdateTime(long long time)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastUpdateTimeKey, time);
    settings->sync();
    mutex.unlock();
}

int Preferences::lastUpdateVersion()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(lastUpdateVersionKey, 0).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setLastUpdateVersion(int version)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastUpdateVersionKey, version);
    settings->sync();
    mutex.unlock();
}

QString Preferences::downloadFolder()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(downloadFolderKey).toString();
    mutex.unlock();
    return value;
}

void Preferences::setDownloadFolder(QString value)
{
    mutex.lock();
    assert(logged());
	settings->setValue(downloadFolderKey, value);
	settings->sync();
    mutex.unlock();
}

long long Preferences::uploadFolder()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(uploadFolderKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setUploadFolder(long long value)
{
    mutex.lock();
    assert(logged());
	settings->setValue(uploadFolderKey, value);
	settings->sync();
    mutex.unlock();
}

long long Preferences::importFolder()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(importFolderKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setImportFolder(long long value)
{
    mutex.lock();
    assert(logged());
	settings->setValue(importFolderKey, value);
	settings->sync();
    mutex.unlock();
}

int Preferences::getNumSyncedFolders()
{
    mutex.lock();
    assert(logged());
    int value = localFolders.length();
    mutex.unlock();
    return value;
}

QString Preferences::getSyncName(int num)
{
    mutex.lock();
    assert(logged() && (syncNames.size()>num));
    if(num >= syncNames.size())
    {
        mutex.unlock();
        return QString();
    }
    QString value = syncNames.at(num);
    mutex.unlock();
    return value;
}

QString Preferences::getLocalFolder(int num)
{
    mutex.lock();
    assert(logged() && (localFolders.size()>num));
    if(num >= localFolders.size())
    {
        mutex.unlock();
        return QString();
    }
    QString value = QDir::toNativeSeparators(localFolders.at(num));
    mutex.unlock();
    return value;
}

QString Preferences::getMegaFolder(int num)
{
    mutex.lock();
    assert(logged() && (megaFolders.size()>num));
    if(num >= megaFolders.size())
    {
        mutex.unlock();
        return QString();
    }
    QString value = megaFolders.at(num);
    mutex.unlock();
    return value;
}

long long Preferences::getMegaFolderHandle(int num)
{
    mutex.lock();
    assert(logged() && (megaFolderHandles.size()>num));
    if(num >= megaFolderHandles.size())
    {
        mutex.unlock();
        return mega::UNDEF;
    }
    long long value = megaFolderHandles.at(num);
    mutex.unlock();
    return value;
}

QStringList Preferences::getSyncNames()
{
    mutex.lock();
    QStringList value = syncNames;
    mutex.unlock();
    return value;
}

QStringList Preferences::getMegaFolders()
{
    mutex.lock();
    QStringList value = megaFolders;
    mutex.unlock();
    return value;
}

QStringList Preferences::getLocalFolders()
{
    mutex.lock();
    QStringList value = localFolders;
    mutex.unlock();
    return value;
}

QList<long long> Preferences::getMegaFolderHandles()
{
    mutex.lock();
    QList<long long> value = megaFolderHandles;
    mutex.unlock();
    return value;
}

void Preferences::addSyncedFolder(QString localFolder, QString megaFolder, long long megaFolderHandle, QString syncName)
{
    mutex.lock();
    assert(logged());
    if(syncName.isEmpty()) syncName = QFileInfo(localFolder).fileName();
    syncNames.append(syncName);
    localFolders.append(QDir::toNativeSeparators(localFolder));
    megaFolders.append(megaFolder);
    megaFolderHandles.append(megaFolderHandle);
    writeFolders();
    mutex.unlock();
    Platform::syncFolderAdded(localFolder, syncName);
}

void Preferences::removeSyncedFolder(int num)
{
    mutex.lock();
    assert(logged());
    syncNames.removeAt(num);
    localFolders.removeAt(num);
    megaFolders.removeAt(num);
    megaFolderHandles.removeAt(num);
    writeFolders();
    mutex.unlock();
}

void Preferences::removeAllFolders()
{
    mutex.lock();
    assert(logged());
    for(int i=0; i<localFolders.size(); i++)
        Platform::syncFolderRemoved(localFolders[i]);

    syncNames.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    writeFolders();
    mutex.unlock();
}

void Preferences::addRecentFile(QString fileName, long long fileHandle, QString localPath)
{
    mutex.lock();
    assert(logged());

    recentFileNames.pop_back();
    recentFileHandles.pop_back();
    recentLocalPaths.pop_back();
    recentFileTime.pop_back();

    recentFileNames.insert(0, fileName);
    recentFileHandles.insert(0, fileHandle);
    recentLocalPaths.insert(0, localPath);
    recentFileTime.insert(0, QDateTime::currentDateTime().toMSecsSinceEpoch());

    writeRecentFiles();
    mutex.unlock();
}

QString Preferences::getRecentFileName(int num)
{
    mutex.lock();
    assert(logged() && recentFileNames.size()>num);
    QString value = recentFileNames[num];
    mutex.unlock();
    return value;
}

long long Preferences::getRecentFileHandle(int num)
{
    mutex.lock();
    assert(logged() && recentFileHandles.size()>num);
    long long value = recentFileHandles[num];
    mutex.unlock();
    return value;
}

QString Preferences::getRecentLocalPath(int num)
{
    mutex.lock();
    assert(logged() && recentLocalPaths.size()>num);
    QString value = recentLocalPaths[num];
    mutex.unlock();
    return value;
}

long long Preferences::getRecentFileTime(int num)
{
    mutex.lock();
    assert(logged() && recentLocalPaths.size()>num);
    long long value = recentFileTime[num];
    mutex.unlock();
    return value;
}

QStringList Preferences::getExcludedSyncNames()
{
    mutex.lock();
    assert(logged());
    QStringList value = excludedSyncNames;
    mutex.unlock();
    return value;
}

void Preferences::setExcludedSyncNames(QStringList names)
{
    mutex.lock();
    assert(logged());
    excludedSyncNames = names;
    if(!excludedSyncNames.size())
        settings->remove(excludedSyncNamesKey);
    else
        settings->setValue(excludedSyncNamesKey, excludedSyncNames.join(QString::fromAscii("\n")));
    settings->sync();
    mutex.unlock();
}

QStringList Preferences::getPreviousCrashes()
{
    mutex.lock();
    QStringList previousCrashes;
    QString currentAccount;
    if(logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }
    previousCrashes = settings->value(previousCrashesKey).toString().split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if(!currentAccount.isEmpty())
        settings->beginGroup(currentAccount);
    mutex.unlock();
    return previousCrashes;
}

void Preferences::setPreviousCrashes(QStringList crashes)
{
    mutex.lock();
    QString currentAccount;
    if(logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }
    if(!crashes.size())
        settings->remove(previousCrashesKey);
    else
        settings->setValue(previousCrashesKey, crashes.join(QString::fromAscii("\n")));
    if(!currentAccount.isEmpty())
        settings->beginGroup(currentAccount);
    settings->sync();
    mutex.unlock();
}

long long Preferences::getLastReboot()
{
    mutex.lock();
    QString currentAccount;
    if(logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    long long value = settings->value(lastRebootKey).toLongLong();

    if(!currentAccount.isEmpty())
        settings->beginGroup(currentAccount);
    mutex.unlock();
    return value;
}

void Preferences::setLastReboot(long long value)
{
    mutex.lock();
    QString currentAccount;
    if(logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(lastRebootKey, value);

    if(!currentAccount.isEmpty())
        settings->beginGroup(currentAccount);
    settings->sync();
    mutex.unlock();
}

long long Preferences::getLastExit()
{
    mutex.lock();
    QString currentAccount;
    if(logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    long long value = settings->value(lastExitKey).toLongLong();

    if(!currentAccount.isEmpty())
        settings->beginGroup(currentAccount);
    mutex.unlock();
    return value;
}

void Preferences::setLastExit(long long value)
{
    mutex.lock();
    QString currentAccount;
    if(logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(lastExitKey, value);

    if(!currentAccount.isEmpty())
        settings->beginGroup(currentAccount);
    settings->sync();
    mutex.unlock();
}

int Preferences::getNumUsers()
{
    mutex.lock();
    assert(!logged());
    int value = settings->numChildGroups();
    mutex.unlock();
    return value;
}

void Preferences::enterUser(int i)
{
    mutex.lock();
    assert(!logged());
    assert(i<settings->numChildGroups());
    if(i<settings->numChildGroups())
        settings->beginGroup(i);
    readFolders();
    readRecentFiles();
    loadExcludedSyncNames();
    mutex.unlock();
}

void Preferences::leaveUser()
{
    mutex.lock();
    assert(logged());
    settings->endGroup();

    syncNames.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    recentFileNames.clear();
    recentFileHandles.clear();
    recentLocalPaths.clear();
    recentFileTime.clear();
    mutex.unlock();
}

void Preferences::unlink()
{
    mutex.lock();
    assert(logged());
    settings->remove(emailHashKey);
    settings->remove(privatePwKey);
    settings->endGroup();

    settings->remove(currentAccountKey);
    syncNames.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    recentFileNames.clear();
    recentFileHandles.clear();
    recentLocalPaths.clear();
    recentFileTime.clear();
    mutex.unlock();
}

bool Preferences::isCrashed()
{
    mutex.lock();
    bool value = settings->value(isCrashedKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setCrashed(bool value)
{
    mutex.lock();
    settings->setValue(isCrashedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::wasPaused()
{
    mutex.lock();
    bool value = settings->value(wasPausedKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setWasPaused(bool value)
{
    mutex.lock();
    settings->setValue(wasPausedKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lastStatsRequest()
{
    mutex.lock();
    long long value = settings->value(lastStatsRequestKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setLastStatsRequest(long long value)
{
    mutex.lock();
    settings->setValue(lastStatsRequestKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::login(QString account)
{
    mutex.lock();
    logout();
    settings->setValue(currentAccountKey, account);
    settings->beginGroup(account);
    readFolders();
    readRecentFiles();
    loadExcludedSyncNames();
    int lastVersion = settings->value(lastVersionKey).toInt();
    if(lastVersion != MegaApplication::VERSION_CODE)
    {
        if((lastVersion != 0) && (lastVersion < MegaApplication::VERSION_CODE))
            ((MegaApplication *)qApp)->showUpdatedMessage();
        settings->setValue(lastVersionKey, MegaApplication::VERSION_CODE);
    }
    settings->sync();
    mutex.unlock();
}

bool Preferences::logged()
{
    mutex.lock();
    bool value = !settings->isGroupEmpty();
    mutex.unlock();
    return value;
}

bool Preferences::hasEmail(QString email)
{
    mutex.lock();
    assert(!logged());
    bool value = settings->containsGroup(email);
    mutex.unlock();
    return value;
}

void Preferences::logout()
{
    mutex.lock();
    if(logged()) settings->endGroup();
    syncNames.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    recentFileNames.clear();
    recentFileHandles.clear();
    recentLocalPaths.clear();
    recentFileTime.clear();
    mutex.unlock();
}

void Preferences::loadExcludedSyncNames()
{
    mutex.lock();
    excludedSyncNames = settings->value(excludedSyncNamesKey).toString().split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if(excludedSyncNames.size()==1 && excludedSyncNames.at(0).isEmpty())
        excludedSyncNames.clear();

    if((settings->value(lastVersionKey).toInt() < 108) &&
       (MegaApplication::VERSION_CODE >= 108))
    {
        excludedSyncNames.clear();
        excludedSyncNames.append(QString::fromUtf8("Thumbs.db"));
        excludedSyncNames.append(QString::fromUtf8("desktop.ini"));
        excludedSyncNames.append(QString::fromUtf8("~*"));
        excludedSyncNames.append(QString::fromUtf8(".*"));
    }

    QMap<QString, QString> strMap;
    foreach ( QString str, excludedSyncNames ) {
        strMap.insert( str.toLower(), str );
    }
    excludedSyncNames = strMap.values();
    setExcludedSyncNames(excludedSyncNames);
    mutex.unlock();
}

void Preferences::readFolders()
{
    mutex.lock();
    assert(logged());
    syncNames.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();

    settings->beginGroup(syncsGroupKey);
    int numSyncs = settings->numChildGroups();
    for(int i=0; i<numSyncs; i++)
    {
        settings->beginGroup(QString::number(i));
            syncNames.append(settings->value(syncNameKey).toString());
            localFolders.append(settings->value(localFolderKey).toString());
            megaFolders.append(settings->value(megaFolderKey).toString());
            megaFolderHandles.append(settings->value(megaFolderHandleKey).toLongLong());
        settings->endGroup();
    }
    settings->endGroup();
    mutex.unlock();
}

void Preferences::writeFolders()
{
    mutex.lock();
    assert(logged());

    settings->beginGroup(syncsGroupKey);
        settings->remove(QString::fromAscii(""));
        for(int i=0; i<localFolders.size(); i++)
        {
            settings->beginGroup(QString::number(i));
                settings->setValue(syncNameKey, syncNames[i]);
                settings->setValue(localFolderKey, localFolders[i]);
                settings->setValue(megaFolderKey, megaFolders[i]);
                settings->setValue(megaFolderHandleKey, megaFolderHandles[i]);
            settings->endGroup();
        }
    settings->endGroup();
    settings->sync();
    mutex.unlock();
}

void Preferences::readRecentFiles()
{
    mutex.lock();
    assert(logged());
    recentFileNames.clear();
    recentFileHandles.clear();
    recentLocalPaths.clear();
    recentFileTime.clear();

    settings->beginGroup(recentGroupKey);
    for(int i=0; i<NUM_RECENT_ITEMS; i++)
    {
        settings->beginGroup(QString::number(i));
            recentFileNames.append(settings->value(fileNameKey).toString());
            recentFileHandles.append(settings->value(fileHandleKey).toLongLong());
            recentLocalPaths.append(settings->value(localPathKey).toString());
            recentFileTime.append(settings->value(fileTimeKey, QDateTime::currentDateTime().toMSecsSinceEpoch()).toLongLong());
        settings->endGroup();
    }
    settings->endGroup();
    mutex.unlock();
}

void Preferences::writeRecentFiles()
{
    mutex.lock();
    assert(logged());
    settings->beginGroup(recentGroupKey);
    for(int i=0; i<NUM_RECENT_ITEMS; i++)
    {
        settings->beginGroup(QString::number(i));
            settings->setValue(fileNameKey, recentFileNames[i]);
            settings->setValue(fileHandleKey, recentFileHandles[i]);
            settings->setValue(localPathKey, recentLocalPaths[i]);
            settings->setValue(fileTimeKey, recentFileTime[i]);
        settings->endGroup();
    }
    settings->endGroup();
    settings->sync();
    mutex.unlock();
}
