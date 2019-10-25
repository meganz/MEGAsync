#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QHash>
#include <QPixmap>
#include <QDir>
#include <QIcon>

#include <sys/stat.h>

#ifdef __APPLE__
#define MEGA_SET_PERMISSIONS chmod("/Applications/MEGAsync.app/Contents/MacOS/MEGAclient", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); \
                             chmod("/Applications/MEGAsync.app/Contents/MacOS/MEGAupdater", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); \
                             chmod("/Applications/MEGAsync.app/Contents/MacOS/MEGADeprecatedVersion", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); \
                             chmod("/Applications/MEGAsync.app/Contents/PlugIns/MEGAShellExtFinder.appex/Contents/MacOS/MEGAShellExtFinder", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

struct PlanInfo
{
    int amount;
    QString currency;
    int gbStorage;
    int gbTransfer;
    int level;
};

struct PSA_info
{
    int idPSA;
    QString title;
    QString desc;
    QString urlImage;
    QString textButton;
    QString urlClick;

    PSA_info()
    {
        clear();
    }

    PSA_info(const PSA_info& info)
    {
        idPSA = info.idPSA;
        title = info.title;
        desc = info.desc;
        urlImage = info.urlImage;
        textButton = info.textButton;
        urlClick = info.urlClick;
    }

    void clear()
    {
        idPSA = -1;
        title = QString();
        desc = QString();
        urlImage = QString();
        textButton = QString();
        urlClick = QString();
    }
};

class IObserver
{
public:
    virtual ~IObserver() = default;
    virtual void update() = 0;
};

class StorageDetailsObserved
{
public:
    virtual ~StorageDetailsObserved() = default;
    void attachStorageObserver(IObserver& obs)
    {
        storageObservers.push_back(&obs);
    }
    void dettachStorageObserver(IObserver& obs)
    {
        storageObservers.erase(std::remove(storageObservers.begin(), storageObservers.end(), &obs));
    }

    void notifyStorageObservers()
    {
        for (IObserver* o : storageObservers)
        {
            o->update();
        }
    }

private:
    std::vector<IObserver*> storageObservers;
};


class Utilities
{
public:
    static QString getSizeString(unsigned long long bytes);
    static QString getTimeString(long long secs, bool secondPrecision = true);
    static QString getFinishedTimeString(long long secs);
    static bool verifySyncedFolderLimits(QString path);
    static QString extractJSONString(QString json, QString name);
    static long long extractJSONNumber(QString json, QString name);
    static QString getDefaultBasePath();

private:
    Utilities() {}
    static QHash<QString, QString> extensionIcons;
    static QHash<QString, QString> languageNames;
    static void initializeExtensions();
    static QString getExtensionPixmapNameSmall(QString fileName);
    static QString getExtensionPixmapNameMedium(QString fileName);

//Platform dependent functions
public:
    static QString languageCodeToString(QString code);
    static QString getAvatarPath(QString email);
    static bool removeRecursively(QString path);
    static void copyRecursively(QString srcPath, QString dstPath);
    static void getFolderSize(QString folderPath, long long *size);
    static qreal getDevicePixelRatio();

    static QIcon getCachedPixmap(QString fileName);
    static QIcon getExtensionPixmapSmall(QString fileName);
    static QIcon getExtensionPixmapMedium(QString fileName);
    static QString getExtensionPixmapName(QString fileName, QString prefix);
};

#endif // UTILITIES_H
