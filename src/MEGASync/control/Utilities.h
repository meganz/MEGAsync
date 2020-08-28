#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QHash>
#include <QPixmap>
#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QEasingCurve>
#include "megaapi.h"

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
    long long gbStorage;
    long long gbTransfer;
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

class IStorageObserver
{
public:
    virtual ~IStorageObserver() = default;
    virtual void updateStorageElements() = 0;
};

class IBandwidthObserver
{
public:
    virtual ~IBandwidthObserver() = default;
    virtual void updateBandwidthElements() = 0;
};

class IAccountObserver
{
public:
    virtual ~IAccountObserver() = default;
    virtual void updateAccountElements() = 0;
};

class StorageDetailsObserved
{
public:
    virtual ~StorageDetailsObserved() = default;
    void attachStorageObserver(IStorageObserver& obs)
    {
        storageObservers.push_back(&obs);
    }
    void dettachStorageObserver(IStorageObserver& obs)
    {
        storageObservers.erase(std::remove(storageObservers.begin(), storageObservers.end(), &obs));
    }

    void notifyStorageObservers()
    {
        for (IStorageObserver* o : storageObservers)
        {
            o->updateStorageElements();
        }
    }

private:
    std::vector<IStorageObserver*> storageObservers;
};

class BandwidthDetailsObserved
{
public:
    virtual ~BandwidthDetailsObserved() = default;
    void attachBandwidthObserver(IBandwidthObserver& obs)
    {
        bandwidthObservers.push_back(&obs);
    }
    void dettachBandwidthObserver(IBandwidthObserver& obs)
    {
        bandwidthObservers.erase(std::remove(bandwidthObservers.begin(), bandwidthObservers.end(), &obs));
    }

    void notifyBandwidthObservers()
    {
        for (IBandwidthObserver* o : bandwidthObservers)
        {
            o->updateBandwidthElements();
        }
    }

private:
    std::vector<IBandwidthObserver*> bandwidthObservers;
};


class AccountDetailsObserved
{
public:
    virtual ~AccountDetailsObserved() = default;
    void attachAccountObserver(IAccountObserver& obs)
    {
        accountObservers.push_back(&obs);
    }
    void dettachAccountObserver(IAccountObserver& obs)
    {
        accountObservers.erase(std::remove(accountObservers.begin(), accountObservers.end(), &obs));
    }

    void notifyAccountObservers()
    {
        for (IAccountObserver* o : accountObservers)
        {
            o->updateAccountElements();
        }
    }

private:
    std::vector<IAccountObserver*> accountObservers;
};


class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(parent)
    {
#ifndef __APPLE__
        setMouseTracking(true);
#endif
    }

    ~ClickableLabel() {}

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event)
    {
        emit clicked();
    }
#ifndef __APPLE__
    void enterEvent(QEvent *event)
    {
        setCursor(Qt::PointingHandCursor);
    }

    void leaveEvent(QEvent *event)
    {
        setCursor(Qt::ArrowCursor);
    }
#endif

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
    static void getPROurlWithParameters(QString &url);
    static QString joinLogZipFiles(mega::MegaApi *megaApi, const QDateTime *timestampSince = nullptr, QString appendHashReference = QString());

    static void adjustToScreenFunc(QPoint position, QWidget *what);
    static QString minProPlanNeeded(mega::MegaPricing *pricing, long long usedStorage);
    static QString getReadableStringFromTs(mega::MegaIntegerList* list);
    static QString getReadablePROplanFromId(int identifier);
    static void animatePartialFadeout(QWidget *object, int msecs = 2000);
    static void animatePartialFadein(QWidget *object, int msecs = 2000);
    static void animateProperty(QWidget *object, int msecs, const char *property, QVariant startValue, QVariant endValue, QEasingCurve curve = QEasingCurve::InOutQuad);
    static void getDaysToTimestamp(int64_t msecsTimestamps, int64_t &remaininDays);
    static void getDaysAndHoursToTimestamp(int64_t msecsTimestamps, int64_t &remaininDays, int64_t &remainingHours);

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

    static long long getSystemsAvailableMemory();
};

#endif // UTILITIES_H
