#include "Preferences.h"
#include "Utils.h"

#include <QApplication>

const int Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER     = 1000;
const int Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER   = 200;

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
const QString Preferences::lastExecutionTimeKey     = QString::fromAscii("lastExecutionTime");

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

Preferences::Preferences()
{
    settings = new EncryptedSettings(qApp->organizationName(), qApp->applicationName());
    locale = new QLocale();

    QString currentAccount = settings->value(currentAccountKey).toString();
    if(currentAccount.size()) login(currentAccount);
}

QString Preferences::email()
{
    assert(logged());

    return settings->value(emailKey).toString();
}

void Preferences::setEmail(QString email)
{
    login(email);
    settings->setValue(emailKey, email);
    settings->sync();
}

QString Preferences::emailHash()
{
    assert(logged());

    return settings->value(emailHashKey).toString();
}

QString Preferences::privatePw()
{
    assert(logged());

    return settings->value(privatePwKey).toString();
}

void Preferences::setCredentials(QString emailHash, QString privatePw)
{
    assert(logged());

    settings->setValue(emailHashKey, emailHash);
    settings->setValue(privatePwKey, privatePw);
    settings->sync();
}

long long Preferences::totalStorage()
{
    assert(logged());

    return settings->value(totalStorageKey).toLongLong();
}

void Preferences::setTotalStorage(long long value)
{
    assert(logged());

    settings->setValue(totalStorageKey, value);
    settings->sync();
}

long long Preferences::usedStorage()
{
    assert(logged());

    return settings->value(usedStorageKey).toLongLong();
}

void Preferences::setUsedStorage(long long value)
{
    assert(logged());

    settings->setValue(usedStorageKey, value);
	settings->sync();
}

long long Preferences::totalBandwidth()
{
    assert(logged());

	return settings->value(totalBandwidthKey).toLongLong();
}

void Preferences::setTotalBandwidth(long long value)
{
    assert(logged());

	settings->setValue(totalBandwidthKey, value);
	settings->sync();
}

long long Preferences::usedBandwidth()
{
    assert(logged());

	return settings->value(usedBandwidthKey).toLongLong();
}

void Preferences::setUsedBandwidth(long long value)
{
    assert(logged());

	settings->setValue(usedBandwidthKey, value);
	settings->sync();
}

int Preferences::accountType()
{
    assert(logged());

    return settings->value(accountTypeKey).toInt();
}

void Preferences::setAccountType(int value)
{
    assert(logged());

    settings->setValue(accountTypeKey, value);
    settings->sync();
}

bool Preferences::showNotifications()
{
    assert(logged());

    return settings->value(showNotificationsKey, defaultShowNotifications).toBool();
}

void Preferences::setShowNotifications(bool value)
{
    assert(logged());

    settings->setValue(showNotificationsKey, value);
    settings->sync();
}

bool Preferences::startOnStartup()
{
    return settings->value(startOnStartupKey, defaultStartOnStartup).toBool();
}

void Preferences::setStartOnStartup(bool value)
{
    settings->setValue(startOnStartupKey, value);
    settings->sync();
}

QString Preferences::language()
{
    return settings->value(languageKey, locale->name()).toString();
}

void Preferences::setLanguage(QString &value)
{
    settings->setValue(startOnStartupKey, value);
    settings->sync();
}

bool Preferences::updateAutomatically()
{
    return settings->value(updateAutomaticallyKey, defaultUpdateAutomatically).toBool();
}

void Preferences::setUpdateAutomatically(bool value)
{
    settings->setValue(updateAutomaticallyKey, value);
    settings->sync();
}

int Preferences::uploadLimitKB()
{
    assert(logged());

    return settings->value(uploadLimitKBKey, defaultUploadLimitKB).toInt();
}

void Preferences::setUploadLimitKB(int value)
{
    assert(logged());

    settings->setValue(uploadLimitKBKey, value);
    settings->sync();
}

int Preferences::proxyType()
{
    assert(logged());

    return settings->value(proxyTypeKey, defaultProxyType).toInt();
}

void Preferences::setProxyType(int value)
{
    assert(logged());

    settings->setValue(proxyTypeKey, value);
    settings->sync();
}

int Preferences::proxyProtocol()
{
    assert(logged());

    return settings->value(proxyProtocolKey, defaultProxyProtocol).toInt();
}

void Preferences::setProxyProtocol(int value)
{
    assert(logged());

    settings->setValue(proxyProtocolKey, value);
    settings->sync();
}

QString Preferences::proxyServer()
{
    assert(logged());

    return settings->value(proxyServerKey, defaultProxyServer).toString();
}

void Preferences::setProxyServer(const QString &value)
{
    assert(logged());

    settings->setValue(proxyServerKey, value);
    settings->sync();
}

int Preferences::proxyPort()
{
    assert(logged());

    return settings->value(proxyPortKey, defaultProxyPort).toInt();
}

void Preferences::setProxyPort(int value)
{
    assert(logged());

    settings->setValue(proxyPortKey, value);
    settings->sync();
}

bool Preferences::proxyRequiresAuth()
{
    assert(logged());

    return settings->value(proxyRequiresAuthKey, defaultProxyRequiresAuth).toBool();
}

void Preferences::setProxyRequiresAuth(bool value)
{
    assert(logged());

    settings->setValue(proxyRequiresAuthKey, value);
    settings->sync();
}

QString Preferences::getProxyUsername()
{
    assert(logged());

    return settings->value(proxyUsernameKey, defaultProxyUsername).toString();
}

void Preferences::setProxyUsername(const QString &value)
{
    assert(logged());

    settings->setValue(proxyUsernameKey, value);
    settings->sync();
}

QString Preferences::getProxyPassword()
{
    assert(logged());

    return settings->value(proxyPasswordKey, defaultProxyPassword).toString();
}

void Preferences::setProxyPassword(const QString &value)
{
    assert(logged());

    settings->setValue(proxyPasswordKey, value);
    settings->sync();
}

long long Preferences::lastExecutionTime()
{
    return settings->value(lastExecutionTimeKey, 0).toLongLong();
}

void Preferences::setLastExecutionTime(qint64 time)
{
    settings->setValue(lastExecutionTimeKey, time);
    settings->sync();
}

QString Preferences::downloadFolder()
{
    assert(logged());

	return settings->value(downloadFolderKey).toString();
}

void Preferences::setDownloadFolder(QString value)
{
    assert(logged());

	settings->setValue(downloadFolderKey, value);
	settings->sync();
}

long long Preferences::uploadFolder()
{
    assert(logged());

	return settings->value(uploadFolderKey).toLongLong();
}

void Preferences::setUploadFolder(long long value)
{
    assert(logged());

	settings->setValue(uploadFolderKey, value);
	settings->sync();
}

long long Preferences::importFolder()
{
    assert(logged());

	return settings->value(importFolderKey).toLongLong();
}

void Preferences::setImportFolder(long long value)
{
    assert(logged());

	settings->setValue(importFolderKey, value);
	settings->sync();
}

int Preferences::getNumSyncedFolders()
{
    assert(logged());

    return localFolders.length();
}

QString Preferences::getSyncName(int num)
{
    assert(logged());

    return syncNames.at(num);
}

QString Preferences::getLocalFolder(int num)
{
    assert(logged() && localFolders.size()>num);

    return QDir::toNativeSeparators(localFolders.at(num));
}

QString Preferences::getMegaFolder(int num)
{
    assert(logged() && syncNames.size()>num);

    return megaFolders.at(num);
}

long long Preferences::getMegaFolderHandle(int num)
{
    assert(logged() && megaFolderHandles.size()>num);

    return megaFolderHandles.at(num);
}

void Preferences::addSyncedFolder(QString localFolder, QString megaFolder, long long megaFolderHandle, QString syncName)
{
    assert(logged());

    if(syncName.isEmpty()) syncName = QFileInfo(localFolder).fileName();
    syncNames.append(syncName);
    localFolders.append(QDir::toNativeSeparators(localFolder));
    megaFolders.append(megaFolder);
    megaFolderHandles.append(megaFolderHandle);
    writeFolders();
    Utils::syncFolderAdded(localFolder, syncName);
}

void Preferences::removeSyncedFolder(int num)
{
    assert(logged());

    Utils::syncFolderRemoved(localFolders[num]);
    syncNames.removeAt(num);
    localFolders.removeAt(num);
    megaFolders.removeAt(num);
    megaFolderHandles.removeAt(num);

    writeFolders();
}

void Preferences::removeAllFolders()
{
    assert(logged());

    for(int i=0; i<localFolders.size(); i++)
        Utils::syncFolderRemoved(localFolders[i]);

    syncNames.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();

    writeFolders();
}

void Preferences::addRecentFile(QString fileName, long long fileHandle, QString localPath)
{
    assert(logged());

    recentFileNames.pop_back();
    recentFileHandles.pop_back();
    recentLocalPaths.pop_back();

    recentFileNames.insert(0, fileName);
    recentFileHandles.insert(0, fileHandle);
    recentLocalPaths.insert(0, localPath);

    writeRecentFiles();
}

QString Preferences::getRecentFileName(int num)
{
    assert(logged() && recentFileNames.size()>num);

    return recentFileNames[num];
}

long long Preferences::getRecentFileHandle(int num)
{
    assert(logged() && recentFileHandles.size()>num);

    return recentFileHandles[num];
}

QString Preferences::getRecentLocalPath(int num)
{
    assert(logged() && recentLocalPaths.size()>num);

    return recentLocalPaths[num];
}

void Preferences::unlink()
{
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
}

void Preferences::login(QString account)
{
    logout();
    settings->setValue(currentAccountKey, account);
    settings->beginGroup(account);
    readFolders();
    readRecentFiles();
}

bool Preferences::logged()
{
    if(settings->isGroupEmpty()) return false;
    return true;
}

bool Preferences::hasEmail(QString email)
{
    assert(!logged());

    return settings->containsGroup(email);
}

void Preferences::logout()
{
    if(logged()) settings->endGroup();
    syncNames.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    recentFileNames.clear();
    recentFileHandles.clear();
    recentLocalPaths.clear();
}

void Preferences::readFolders()
{
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
}

void Preferences::writeFolders()
{
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
}

void Preferences::readRecentFiles()
{
    assert(logged());

    recentFileNames.clear();
    recentFileHandles.clear();
    recentLocalPaths.clear();

    settings->beginGroup(recentGroupKey);
    for(int i=0; i<NUM_RECENT_ITEMS; i++)
    {
        settings->beginGroup(QString::number(i));
            recentFileNames.append(settings->value(fileNameKey).toString());
            recentFileHandles.append(settings->value(fileHandleKey).toLongLong());
            recentLocalPaths.append(settings->value(localPathKey).toString());
        settings->endGroup();
    }
    settings->endGroup();
}

void Preferences::writeRecentFiles()
{
    assert(logged());

    settings->beginGroup(recentGroupKey);
    for(int i=0; i<NUM_RECENT_ITEMS; i++)
    {
        settings->beginGroup(QString::number(i));
            settings->setValue(fileNameKey, recentFileNames[i]);
            settings->setValue(fileHandleKey, recentFileHandles[i]);
            settings->setValue(localPathKey, recentLocalPaths[i]);
        settings->endGroup();
    }
    settings->endGroup();

    settings->sync();
}
