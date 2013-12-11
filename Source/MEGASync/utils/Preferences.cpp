#include "Preferences.h"
#include <QApplication>

const int Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER     = 1000;
const int Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER   = 200;

const QString Preferences::trayIconEnabledKey       = "trayIconEnabled";
const QString Preferences::emailKey                 = "email";
const QString Preferences::passwordKey              = "password";
const QString Preferences::totalStorageKey          = "totalStorage";
const QString Preferences::usedStorageKey           = "usedStorage";
const QString Preferences::totalBandwidthKey		= "totalBandwidth";
const QString Preferences::usedBandwidthKey			= "usedBandwidth";
const QString Preferences::accountTypeKey           = "accountType";
const QString Preferences::setupWizardCompletedKey  = "setupWizardCompleted";
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
const QString Preferences::localFoldersKey          = "localFolders";
const QString Preferences::megaFoldersKey           = "megaFolders";
const QString Preferences::megaFolderHandlesKey     = "megaFolderHandles";
const QString Preferences::downloadFolderKey		= "downloadFolder";
const QString Preferences::uploadFolderKey			= "uploadFolder";
const QString Preferences::importFolderKey			= "importFolder";

const bool Preferences::defaultSetupWizardCompleted = false;
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

Preferences::Preferences()
{
    settings = new QSettings(qApp->organizationName(), qApp->applicationName());
    locale = new QLocale();
    qRegisterMetaType<QList<long long> >("QListLongLong");
    qRegisterMetaTypeStreamOperators<QList<long long> >("QListLongLong");

	//clearAll();

    readFolders();
}

bool Preferences::isTrayIconEnabled()
{
	return settings->value(trayIconEnabledKey, true).toBool();
}

void Preferences::setTrayIconEnabled(bool value)
{
    settings->setValue(trayIconEnabledKey, value);
    settings->sync();
}

QString Preferences::email()
{
    return settings->value(emailKey).toString();
}

void Preferences::setEmail(QString email)
{
    settings->setValue(emailKey, email);
    settings->sync();
}

QString Preferences::password()
{
    return settings->value(passwordKey).toString();
}

void Preferences::setPassword(QString password)
{
    settings->setValue(passwordKey, password);
    settings->sync();
}

long long Preferences::totalStorage()
{
    return settings->value(totalStorageKey).toLongLong();
}

void Preferences::setTotalStorage(long long value)
{
    settings->setValue(totalStorageKey, value);
    settings->sync();
}

long long Preferences::usedStorage()
{
    return settings->value(usedStorageKey).toLongLong();
}

void Preferences::setUsedStorage(long long value)
{
    settings->setValue(usedStorageKey, value);
	settings->sync();
}

long long Preferences::totalBandwidth()
{
	return settings->value(totalBandwidthKey).toLongLong();
}

void Preferences::setTotalBandwidth(long long value)
{
	settings->setValue(totalBandwidthKey, value);
	settings->sync();
}

long long Preferences::usedBandwidth()
{
	return settings->value(usedBandwidthKey).toLongLong();
}

void Preferences::setUsedBandwidth(long long value)
{
	settings->setValue(usedBandwidthKey, value);
	settings->sync();
}

int Preferences::accountType()
{
    return settings->value(accountTypeKey).toInt();
}

void Preferences::setAccountType(int value)
{
    settings->setValue(accountTypeKey, value);
    settings->sync();
}

bool Preferences::isSetupWizardCompleted()
{
    return settings->value(setupWizardCompletedKey, defaultSetupWizardCompleted).toBool();
}

void Preferences::setSetupWizardCompleted(bool value)
{
    settings->setValue(setupWizardCompletedKey, value);
    settings->sync();
}

bool Preferences::showNotifications()
{
    return settings->value(showNotificationsKey, defaultShowNotifications).toBool();
}

void Preferences::setShowNotifications(bool value)
{
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
    return settings->value(uploadLimitKBKey, defaultUploadLimitKB).toInt();
}

void Preferences::setUploadLimitKB(int value)
{
    settings->setValue(uploadLimitKBKey, value);
    settings->sync();
}

int Preferences::proxyType()
{
    return settings->value(proxyTypeKey, defaultProxyType).toInt();
}

void Preferences::setProxyType(int value)
{
    settings->setValue(proxyTypeKey, value);
    settings->sync();
}

int Preferences::proxyProtocol()
{
    return settings->value(proxyProtocolKey, defaultProxyProtocol).toInt();
}

void Preferences::setProxyProtocol(int value)
{
    settings->setValue(proxyProtocolKey, value);
    settings->sync();
}

QString Preferences::proxyServer()
{
    return settings->value(proxyServerKey, defaultProxyServer).toString();
}

void Preferences::setProxyServer(const QString &value)
{
    settings->setValue(proxyServerKey, value);
    settings->sync();
}

int Preferences::proxyPort()
{
    return settings->value(proxyPortKey, defaultProxyPort).toInt();
}

void Preferences::setProxyPort(int value)
{
    settings->setValue(proxyPortKey, value);
    settings->sync();
}

bool Preferences::proxyRequiresAuth()
{
    return settings->value(proxyRequiresAuthKey, defaultProxyRequiresAuth).toBool();
}

void Preferences::setProxyRequiresAuth(bool value)
{
    settings->setValue(proxyRequiresAuthKey, value);
    settings->sync();
}

QString Preferences::getProxyUsername()
{
    return settings->value(proxyUsernameKey, defaultProxyUsername).toString();
}

void Preferences::setProxyUsername(const QString &value)
{
    settings->setValue(proxyUsernameKey, value);
    settings->sync();
}

QString Preferences::getProxyPassword()
{
    return settings->value(proxyPasswordKey, defaultProxyPassword).toString();
}

void Preferences::setProxyPassword(const QString &value)
{
    settings->setValue(proxyPasswordKey, value);
	settings->sync();
}

QString Preferences::downloadFolder()
{
	return settings->value(downloadFolderKey).toString();
}

void Preferences::setDownloadFolder(QString value)
{
	settings->setValue(downloadFolderKey, value);
	settings->sync();
}

long long Preferences::uploadFolder()
{
	return settings->value(uploadFolderKey).toLongLong();
}

void Preferences::setUploadFolder(long long value)
{
	settings->setValue(uploadFolderKey, value);
	settings->sync();
}

long long Preferences::importFolder()
{
	return settings->value(importFolderKey).toLongLong();
}

void Preferences::setImportFolder(long long value)
{
	settings->setValue(importFolderKey, value);
	settings->sync();
}

int Preferences::getNumSyncedFolders()
{
    if(megaFolders.length() < localFolders.length()) return megaFolders.length();
    return localFolders.length();
}

QString Preferences::getLocalFolder(int num)
{
    return localFolders.at(num);
}

QString Preferences::getMegaFolder(int num)
{
    return megaFolders.at(num);
}

long long Preferences::getMegaFolderHandle(int num)
{
    return megaFolderHandles.at(num);
}

void Preferences::addSyncedFolder(QString localFolder, QString megaFolder, long long megaFolderHandle)
{
    localFolders.append(localFolder);
    megaFolders.append(megaFolder);
    megaFolderHandles.append(megaFolderHandle);
    writeFolders();
}

void Preferences::removeSyncedFolder(int num)
{
    localFolders.removeAt(num);
    megaFolders.removeAt(num);
    megaFolderHandles.removeAt(num);
    writeFolders();
}

void Preferences::removeAllFolders()
{
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    writeFolders();
}

void Preferences::clearAll()
{
	removeAllFolders();
    settings->clear();
	settings->sync();
}

void Preferences::readFolders()
{
    localFolders = settings->value(localFoldersKey).toStringList();
    megaFolders = settings->value(megaFoldersKey).toStringList();
    megaFolderHandles = settings->value(megaFolderHandlesKey).value<QList<long long> >();
}

void Preferences::writeFolders()
{
    settings->setValue(localFoldersKey, localFolders);
    settings->setValue(megaFoldersKey, megaFolders);
    settings->setValue(megaFolderHandlesKey, QVariant::fromValue(megaFolderHandles));
    settings->sync();
}
