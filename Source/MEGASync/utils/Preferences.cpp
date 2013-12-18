#include "Preferences.h"
#include "Utils.h"

#include <QApplication>

const int Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER     = 1000;
const int Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER   = 200;

const QString Preferences::syncsGroupKey            = "Syncs";
const QString Preferences::recentGroupKey           = "Recent";
const QString Preferences::currentAccountKey        = "currentAccount";
const QString Preferences::emailKey                 = "email";
const QString Preferences::emailHashKey             = "emailHash";
const QString Preferences::privatePwKey             = "privatePw";
const QString Preferences::totalStorageKey          = "totalStorage";
const QString Preferences::usedStorageKey           = "usedStorage";
const QString Preferences::totalBandwidthKey		= "totalBandwidth";
const QString Preferences::usedBandwidthKey			= "usedBandwidth";
const QString Preferences::accountTypeKey           = "accountType";
const QString Preferences::showNotificationsKey     = "showNotifications";
const QString Preferences::startOnStartupKey        = "startOnStartup";
const QString Preferences::languageKey              = "language";
const QString Preferences::updateAutomaticallyKey   = "updateAutomatically";
const QString Preferences::uploadLimitKBKey         = "uploadLimitKB";
const QString Preferences::proxyTypeKey             = "proxyType";
const QString Preferences::proxyProtocolKey         = "proxyProtocol";
const QString Preferences::proxyServerKey           = "proxyServer";
const QString Preferences::proxyPortKey             = "proxyPort";
const QString Preferences::proxyRequiresAuthKey     = "proxyRequiresAuth";
const QString Preferences::proxyUsernameKey         = "proxyUsername";
const QString Preferences::proxyPasswordKey         = "proxyPassword";
const QString Preferences::localFolderKey          = "localFolder";
const QString Preferences::megaFolderKey           = "megaFolder";
const QString Preferences::megaFolderHandleKey     = "megaFolderHandle";
const QString Preferences::downloadFolderKey		= "downloadFolder";
const QString Preferences::uploadFolderKey			= "uploadFolder";
const QString Preferences::importFolderKey			= "importFolder";
const QString Preferences::fileNameKey              = "fileName";
const QString Preferences::fileHandleKey            = "fileHandle";
const QString Preferences::localPathKey             = "localPath";

const bool Preferences::defaultShowNotifications    = true;
const bool Preferences::defaultStartOnStartup       = true;
const bool Preferences::defaultUpdateAutomatically  = true;
const int  Preferences::defaultUploadLimitKB        = -1;
const int  Preferences::defaultProxyType            = PROXY_TYPE_AUTO;
const QString  Preferences::defaultProxyProtocol    = "HTTP";
const QString  Preferences::defaultProxyServer      = "127.0.0.1";
const int Preferences::defaultProxyPort             = 8080;
const bool Preferences::defaultProxyRequiresAuth    = false;
const QString Preferences::defaultProxyUsername     = "";
const QString Preferences::defaultProxyPassword     = "";
const int Preferences::NUM_RECENT_ITEMS             = 3;

Preferences::Preferences()
{
    settings = new QSettings(qApp->organizationName(), qApp->applicationName());
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

QString Preferences::getLocalFolder(int num)
{
    assert(logged() && localFolders.size()>num);

    return localFolders.at(num);
}

QString Preferences::getMegaFolder(int num)
{
    assert(logged() && megaFolders.size()>num);

    return megaFolders.at(num);
}

long long Preferences::getMegaFolderHandle(int num)
{
    assert(logged() && megaFolderHandles.size()>num);

    return megaFolderHandles.at(num);
}

void Preferences::addSyncedFolder(QString localFolder, QString megaFolder, long long megaFolderHandle)
{
    assert(logged());

    localFolders.append(localFolder);
    megaFolders.append(megaFolder);
    megaFolderHandles.append(megaFolderHandle);
    writeFolders();
    Utils::syncFolderAdded(localFolder);
}

void Preferences::removeSyncedFolder(int num)
{
    assert(logged());

    Utils::syncFolderRemoved(localFolders[num]);
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
    if(settings->group().isEmpty()) return false;
    return true;
}

bool Preferences::hasEmail(QString email)
{
    assert(!logged());

    return settings->childGroups().contains(email);
}

void Preferences::logout()
{
    if(logged()) settings->endGroup();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
}

void Preferences::readFolders()
{
    assert(logged());

    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();

    settings->beginGroup(syncsGroupKey);
    QStringList syncs = settings->childGroups();
    for(int i=0; i<syncs.size(); i++)
    {
        settings->beginGroup(syncs[i]);
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
        settings->remove("");
        for(int i=0; i<localFolders.size(); i++)
        {
            settings->beginGroup(QString::number(i));
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
