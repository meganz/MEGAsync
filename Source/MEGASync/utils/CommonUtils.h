#ifndef COMMONUTILS_H
#define COMMONUTILS_H

#include "MegaApplication.h"
#include "ShellDispatcherListener.h"

#include <QString>
#include <QHash>
#include <QPixmap>
#include <QDir>

//#define SHOW_LOGS
#ifdef SHOW_LOGS
    #define LOG(x) Utils::log(x)
#else
    #define LOG(X)
#endif

template <class T>
class CommonUtils
{
public:
    static QString getSizeString(unsigned long long bytes);
    static bool verifySyncedFolderLimits(QString path);

private:
    CommonUtils() {}
    static QHash<QString, QString> extensionIcons;
    static void initializeExtensions();
    static void countFilesAndFolders(QString path, long *numFiles, long *numFolders, long fileLimit, long folderLimit);
    static QPixmap getExtensionPixmap(QString fileName, QString prefix);

//Platform dependent functions
public:

    static QPixmap getExtensionPixmapSmall(QString fileName);
    static QPixmap getExtensionPixmapMedium(QString fileName);
    static QImage createThumbnail(QString imagePath, int size);
    static bool removeRecursively(QDir dir);
    static bool enableTrayIcon(QString executable) { return T::enableTrayIcon(executable); }
    static void notifyItemChange(QString path) { T::notifyItemChange(path); }
    static bool startOnStartup(bool value) { return T::startOnStartup(value); }
    static bool isStartOnStartupActive() { return T::isStartOnStartupActive(); }
    static void showInFolder(QString pathIn) { T::showInFolder(pathIn); }
    static void startShellDispatcher(MegaApplication *receiver) { T::startShellDispatcher(receiver); }
    static void stopShellDispatcher() { T::stopShellDispatcher(); }
    static void syncFolderAdded(QString path, QString syncName) { T::syncFolderAdded(path, syncName); }
    static void syncFolderRemoved(QString path) { T::syncFolderRemoved(path); }
    static QByteArray encrypt(QByteArray data, QByteArray key) { return T::encrypt(data, key); }
    static QByteArray decrypt(QByteArray data, QByteArray key) { return T::decrypt(data, key); }
    static QByteArray getLocalStorageKey() { return T::getLocalStorageKey(); }
    static void log(QString message);
    static void log(const char *message);
};

template <class T>
QHash<QString, QString> CommonUtils<T>::extensionIcons;

#endif // COMMONUTILS_H
