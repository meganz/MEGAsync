#ifndef UTILITIES_H
#define UTILITIES_H

#include "megaapi.h"
#include "ThreadPool.h"

#include <QDesktopServices>
#include <QDir>
#include <QEasingCurve>
#include <QEvent>
#include <QEventLoop>
#include <QFuture>
#include <QHash>
#include <QIcon>
#include <QLabel>
#include <QMetaEnum>
#include <QPixmap>
#include <QProgressDialog>
#include <QQueue>
#include <QString>
#include <QTimer>
#include <sys/stat.h>

#include <functional>

#ifdef __APPLE__
#define MEGA_SET_PERMISSIONS \
chmod("/Applications/MEGAsync.app/Contents/MacOS/MEGAsync", \
      S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); \
chmod("/Applications/MEGAsync.app/Contents/MacOS/MEGAupdater", \
      S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); \
chmod("/Applications/MEGAsync.app/Contents/MacOS/mega-desktop-app-gfxworker", \
      S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); \
chmod("/Applications/MEGAsync.app/Contents/PlugIns/MEGAShellExtFinder.appex/Contents/MacOS/" \
      "MEGAShellExtFinder", \
      S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

#define MegaSyncApp (static_cast<MegaApplication *>(QCoreApplication::instance()))

template <typename E>
constexpr typename std::underlying_type<E>::type toInt(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

struct PlanInfo
{
    long long gbStorage;
    long long gbTransfer;
    unsigned int minUsers;
    int level;
    int gbPerStorage;
    int gbPerTransfer;
    unsigned int pricePerUserBilling;
    unsigned int pricePerUserLocal;
    unsigned int pricePerStorageBilling;
    unsigned int pricePerStorageLocal;
    unsigned int pricePerTransferBilling;
    unsigned int pricePerTransferLocal;
    QString billingCurrencySymbol;
    QString billingCurrencyName;
    QString localCurrencySymbol;
    QString localCurrencyName;
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

namespace DeviceOs
{
Q_NAMESPACE

enum Os
{
    UNDEFINED,
    LINUX,
    MAC,
    WINDOWS
};
Q_ENUM_NS(Os)

inline DeviceOs::Os getCurrentOS()
{
#ifdef Q_OS_WINDOWS
    return DeviceOs::WINDOWS;
#endif
#ifdef Q_OS_MACOS
    return DeviceOs::MAC;
#endif
#ifdef Q_OS_LINUX
    return DeviceOs::LINUX;
#endif
}
}

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
        storageObservers.erase(std::remove(storageObservers.begin(), storageObservers.end(), &obs), storageObservers.end());
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

class ThreadPoolSingleton
{
    private:
        static std::unique_ptr<ThreadPool> instance;
        ThreadPoolSingleton() {}

    public:
        static ThreadPool* getInstance()
        {
            if (instance == nullptr)
            {
                instance.reset(new ThreadPool(5));
            }

            return instance.get();
        }
};


/**
 * @brief The MegaListenerFuncExecuter class
 *
 * it takes an std::function as parameter that will be called upon request finish.
 *
 */
class MegaListenerFuncExecuter : public mega::MegaRequestListener
{
private:
    bool mAutoremove = true;
    bool mExecuteInAppThread = true;
    std::function<void(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError *e)> onRequestFinishCallback;

public:

    /**
     * @brief MegaListenerFuncExecuter
     * @param func to call upon onRequestFinish
     * @param autoremove whether this should be deleted after func is called
     */
    MegaListenerFuncExecuter(bool autoremove = false,
                             std::function<void(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError *e)> func = nullptr
                            )
        : mAutoremove(autoremove), onRequestFinishCallback(std::move(func))
    {
    }

    virtual void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e);
    virtual void onRequestStart(mega::MegaApi*, mega::MegaRequest*) {}
    virtual void onRequestUpdate(mega::MegaApi*, mega::MegaRequest*) {}
    virtual void onRequestTemporaryError(mega::MegaApi*, mega::MegaRequest*, mega::MegaError*) {}

    void setExecuteInAppThread(bool executeInAppThread);
};

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(parent)
    {
        Q_UNUSED(f)
#ifndef __APPLE__
        setMouseTracking(true);
#endif
    }

    ~ClickableLabel() {}

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent*)
    {
        emit clicked();
    }
#ifndef __APPLE__
    void enterEvent(QEvent*)
    {
        setCursor(Qt::PointingHandCursor);
    }

    void leaveEvent(QEvent*)
    {
        setCursor(Qt::ArrowCursor);
    }
#endif
};

struct TimeInterval
{
    TimeInterval(long long secs, bool secondPrecision = true);

    TimeInterval& operator=(const TimeInterval& other);

    int days;
    int hours;
    int minutes;
    int seconds;
    bool useSecondPrecision;
};

class Utilities : public QObject
{
    Q_OBJECT

public:
    enum class AttributeType
    {
        NONE = 0x0,
        SMALL = 0x1,
        MEDIUM = 0x2,
        DISABLED = 0x4
    };
    Q_DECLARE_FLAGS(AttributeTypes, AttributeType)
    Q_ENUM(AttributeType)
    Q_ENUM(AttributeTypes)

    enum class FileType
    {
        TYPE_OTHER    = 0x01,
        TYPE_AUDIO    = 0x02,
        TYPE_VIDEO    = 0x04,
        TYPE_ARCHIVE  = 0x08,
        TYPE_DOCUMENT = 0x10,
        TYPE_IMAGE    = 0x20,
    };
    Q_DECLARE_FLAGS(FileTypes, FileType)

    enum class FolderType
    {
        TYPE_NORMAL,
        TYPE_BACKUP_1,
        TYPE_BACKUP_2,
        TYPE_CAMERA_UPLOADS,
        TYPE_CHAT,
        TYPE_DROP,
        TYPE_INCOMING_SHARE,
        TYPE_OUTGOING_SHARE,
        TYPE_REQUEST,
        TYPE_SYNC,
        TYPE_USERS
    };

    static const QString SUPPORT_URL;
    static const QString BACKUP_CENTER_URL;
    static const QString SYNC_SUPPORT_URL;
    static const QString DESKTOP_APP_URL;

    static QString getSizeString(long long bytes);
    static QString getSizeStringLocalized(qint64 bytes);
    static int toNearestUnit(long long bytes);
    static QString getTranslatedSeparatorTemplate();

    struct ProgressSize
    {
        QString transferredBytes;
        QString totalBytes;
        QString units;
    };
    static ProgressSize getProgressSizes(long long transferredBytes, long long totalBytes);

    static QString createSimpleUsedString(long long usedData);
    static QString createSimpleUsedOfString(long long usedData, long long totalData);
    static QString createSimpleUsedStringWithoutReplacement(long long usedData);
    static QString createCompleteUsedString(long long usedData, long long totalData, int percentage);
    static QString getTimeString(long long secs, bool secondPrecision = true, bool color = true);
    static QString getAddedTimeString(long long secs);
    static QString extractJSONString(QString json, QString name);
    static QStringList extractJSONStringList(const QString& json, const QString& name);
    static long long extractJSONNumber(QString json, QString name);
    static QString getDefaultBasePath();
    static void getPROurlWithParameters(QString &url);
    static QString joinLogZipFiles(mega::MegaApi *megaApi, const QDateTime *timestampSince = nullptr, QString appendHashReference = QString());

    static void adjustToScreenFunc(QPoint position, QWidget* what);
    static QString getReadableStringFromTs(mega::MegaIntegerList* list);
    static QString getReadablePlanFromId(int identifier, bool shortPlan = false);
    static void animateFadeout(QWidget *object, int msecs = 700);
    static void animateFadein(QWidget *object, int msecs = 700);
    static void animatePartialFadeout(QWidget *object, int msecs = 2000);
    static void animatePartialFadein(QWidget *object, int msecs = 2000);
    static void animateProperty(QWidget *object, int msecs, const char *property, QVariant startValue, QVariant endValue, QEasingCurve curve = QEasingCurve::InOutQuad);
    // Returns remaining days until given Unix timestamp in seconds.
    static void getDaysToTimestamp(int64_t secsTimestamps, int64_t &remaininDays);
    // Returns remaining days / hours until given Unix timestamp in seconds.
    // Note: remainingHours and remaininDays represent the same value.
    // i.e. for 1 day & 3 hours remaining, remainingHours will be 27, not 3.
    static void getDaysAndHoursToTimestamp(int64_t secsTimestamps, int64_t &remaininDays, int64_t &remainingHours);

    static QString getNonDuplicatedNodeName(mega::MegaNode* node, mega::MegaNode* parentNode, const QString& currentName, bool unescapeName, const QStringList &itemsBeingRenamed);
    static QString getNonDuplicatedLocalName(const QFileInfo& currentFile, bool unescapeName, const QStringList &itemsBeingRenamed);
    static QPair<QString, QString> getFilenameBasenameAndSuffix(const QString& fileName);

    static void upgradeClicked();

    //get mega transfer nodepath
    static QString getNodePath(mega::MegaTransfer* transfer);

    // Detect folder case sensitivity utils
    static Qt::CaseSensitivity isCaseSensitive(const QString& folder);

    //Check is current account is business (either business or flexi pro)
    static bool isBusinessAccount();
    static QFuture<bool> openUrl(QUrl url);
    static void openAppDataPath();
    static void openInMega(mega::MegaHandle handle);
    static void openBackupCenter();

    static QString getCommonPath(const QString& path1, const QString& path2, bool cloudPaths);

    static bool isIncommingShare(mega::MegaNode* node);
    static int getNodeAccess(mega::MegaHandle handle);
    static int getNodeAccess(mega::MegaNode* handle);
    static QString getNodeStringAccess(mega::MegaNode* handle);
    static QString getNodeStringAccess(mega::MegaHandle handle);

    enum HandlesType
    {
        FILES = 0x1,
        FOLDERS = 0x2
    };
    Q_DECLARE_FLAGS(HandlesTypes, HandlesType)

    static HandlesTypes getHandlesType(const QList<mega::MegaHandle>& handles);

    static bool hourHasChanged(qint64 date1, qint64 date2);

    static bool dayHasChangedSince(qint64 msecs);
    static bool monthHasChangedSince(qint64 msecs);

    static QString getTranslatedError(const mega::MegaError* error);

    static std::shared_ptr<mega::MegaError> removeRemoteFile(const mega::MegaNode* node);
    static std::shared_ptr<mega::MegaError> removeSyncRemoteFile(const mega::MegaNode* node);
    static std::shared_ptr<mega::MegaError> removeRemoteFile(const QString& path);
    static std::shared_ptr<mega::MegaError> removeSyncRemoteFile(const QString& path);
    static bool removeLocalFile(const QString& path, const mega::MegaHandle& syncId);
    static bool restoreNode(mega::MegaNode* node,
                            mega::MegaApi* megaApi,
                            bool async,
                            std::function<void(mega::MegaRequest*, mega::MegaError*)> finishFunc);

    static bool restoreNode(const char* nodeName,
                            const char* nodeDirectoryPath,
                            mega::MegaApi* megaApi,
                            std::function<void(mega::MegaRequest*, mega::MegaError*)> finishFunc);

private:
    Utilities() {}
    static QHash<QString, QString> extensionIcons;
    static QMap<FolderType, QString> folderIcons;
    static QHash<QString, FileType> fileTypes;
    static QHash<QString, QString> languageNames;
    static void initializeExtensions();
    static void initializeFileTypes();
    static void initializeFolderTypes();
    static double toDoubleInUnit(unsigned long long bytes, unsigned long long unit);
    static QString getTimeFormat(const TimeInterval& interval);
    static QString filledTimeString(const QString& timeFormat, const TimeInterval& interval, bool color);

    static QString cleanedTimeString(const QString& timeString);

    static long long getNearestUnit(long long bytes);

    // Detect folder case sensitivity utils
    static void createFile(const QDir& path, const QString& filename);
    static uint8_t getFileCount(QDir path);

    // Platform dependent functions

public:
    static QString languageCodeToString(QString code);
    static QString getAvatarPath(QString email);
    static void removeAvatars();
    static bool removeRecursively(QString path);
    static void copyRecursively(QString srcPath, QString dstPath);

    static void queueFunctionInAppThread(std::function<void()> fun);
    static void queueFunctionInObjectThread(QObject* object, std::function<void()> fun);

    static void getFolderSize(QString folderPath, long long *size);
    static qreal getDevicePixelRatio();

    static QIcon getCachedPixmap(QString fileName);
    static QIcon getExtensionPixmap(QString fileName, AttributeTypes attribute);
    static QString getExtensionPixmapName(QString fileName, AttributeTypes attribute);
    static QString getUndecryptedPixmapName(AttributeTypes attribute);
    static QIcon getFolderPixmap(FolderType type, AttributeTypes attribute);
    static QString getFolderPixmapName(FolderType type, AttributeTypes attribute);
    static void clearIconCache();
    static FileType getFileType(QString fileName, AttributeTypes prefix);

    static long long getSystemsAvailableMemory();

    static void sleepMilliseconds(unsigned int milliseconds);

    // Compute the part per <ref> of <part> from <total>. Defaults to %
    static int partPer(long long part, long long total, uint ref = 100);

    static QString getFileHash(const QString& filePath);

    // Human-friendly list of forbidden chars for New Remote Folder
    static const QLatin1String FORBIDDEN_CHARS;
    // Forbidden chars PCRE
    static const QRegularExpression FORBIDDEN_CHARS_RX;
    // Time to show the new remote folder input error in milliseconds
    static constexpr int ERROR_DISPLAY_TIME_MS = 10000; //10s in milliseconds

    static bool isNodeNameValid(const QString& name);

    static bool shouldDisplayUpgradeButton(const bool isTransferOverquota);

    static void propagateCustomEvent(QEvent::Type event);
};

Q_DECLARE_METATYPE(Utilities::FileType)
Q_DECLARE_OPERATORS_FOR_FLAGS(Utilities::FileTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Utilities::AttributeTypes)

// This class encapsulates a MEGA node and adds useful information, like the origin
// of the transfer.
class WrappedNode
{
public:
    // Enum used to record origin of transfer
    enum TransferOrigin
    {
        FROM_UNKNOWN = 0,
        FROM_APP = 1,
        FROM_WEBSERVER = 2,
        FROM_LINK = 3,
    };

    // Constructor with origin and pointer to MEGA node. Default to unknown/nullptr
    // @param undelete Indicates a special request for a node that has been completely deleted
    // (even from Rubbish Bin); allowed only for accounts with PRO level
    WrappedNode(TransferOrigin from = WrappedNode::TransferOrigin::FROM_UNKNOWN,
                mega::MegaNode* node = nullptr,
                bool undelete = false);

    WrappedNode(TransferOrigin from = WrappedNode::TransferOrigin::FROM_UNKNOWN,
                std::shared_ptr<mega::MegaNode> node = nullptr,
                bool undelete = false);

    // Destructor
    ~WrappedNode() {}

    // Get the transfer origin
    WrappedNode::TransferOrigin getTransferOrigin() const
    {
        return mTransfersFrom;
    }

    // Get the wrapped MEGA node pointer
    mega::MegaNode* getMegaNode() const
    {
        return mNode.get();
    }

    bool getUndelete() const
    {
        return mUndelete;
    }

private:
    // Keep track of transfer origin
    WrappedNode::TransferOrigin  mTransfersFrom;

    // Wrapped MEGA node
    std::shared_ptr<mega::MegaNode> mNode;

    bool mUndelete;
};

Q_DECLARE_METATYPE(QQueue<WrappedNode>)

#endif // UTILITIES_H
