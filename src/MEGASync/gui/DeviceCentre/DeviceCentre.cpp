#include "DeviceCentre.h"

#include "CreateRemoveBackupsManager.h"
#include "CreateRemoveSyncsManager.h"
#include "MegaApplication.h"

namespace
{
const uint DELAY_TO_NEXT_CALL_IN_MS = 1000u;
static bool qmlRegistrationDone = false;
}

DeviceCentre::DeviceCentre(QObject* parent):
    QMLComponent(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSyncModel(new SyncModel(this)),
    mDeviceModel(new DeviceModel(this))
{
    registerQmlModules();

    mDelegateListener = std::make_unique<mega::QTMegaListener>(mMegaApi, this);
    mMegaApi->addListener(mDelegateListener.get());

    mSizeInfoTimer.setInterval(DELAY_TO_NEXT_CALL_IN_MS);
    mSizeInfoTimer.setSingleShot(true);
    connect(&mSizeInfoTimer, &QTimer::timeout, this, [this]() {
        mMegaApi->getBackupInfo();
    });
}

DeviceCentre::~DeviceCentre()
{
    mMegaApi->removeListener(mDelegateListener.get());
    mSizeInfoTimer.stop();
}

void DeviceCentre::onRequestFinish(mega::MegaApi* api,
                                   mega::MegaRequest* request,
                                   mega::MegaError* e)
{
    const bool isHandledHere = request->getParamType() == mega::MegaApi::USER_ATTR_DEVICE_NAMES ||
                               request->getType() == mega::MegaRequest::TYPE_BACKUP_INFO ||
                               request->getType() == mega::MegaRequest::TYPE_ADD_SYNC ||
                               request->getType() == mega::MegaRequest::TYPE_REMOVE_SYNC;

    if (e->getErrorCode() == mega::MegaError::API_OK)
    {
        if (request->getParamType() == mega::MegaApi::USER_ATTR_DEVICE_NAMES)
        {
            mCachedDeviceData.name = QString::fromUtf8(request->getName());
            mDeviceModel->reset(mDeviceIdFromLastRequest, mCachedDeviceData);
            emit deviceDataUpdated();
        }
        else if (request->getType() == mega::MegaRequest::TYPE_BACKUP_INFO)
        {
            mega::MegaBackupInfoList* backupList = request->getMegaBackupInfoList();
            updateLocalData(*backupList);

            if (mSyncModel->hasUpdatingStatus())
            {
                mSizeInfoTimer.start();
            }
            emit deviceDataUpdated();
        }
        else if (request->getType() == mega::MegaRequest::TYPE_ADD_SYNC)
        {
            const QmlSyncData syncObject(request, mMegaApi);
            mSyncModel->addOrUpdate(syncObject);

            mMegaApi->getBackupInfo();
            emit deviceDataUpdated();
        }
        else if (request->getType() == mega::MegaRequest::TYPE_REMOVE_SYNC)
        {
            mSyncModel->remove(request->getParentHandle());
            updateDeviceData();
            emit deviceDataUpdated();
        }
    }
    else if (isHandledHere)
    {
        const QString errorMsg = QString::fromUtf8(e->getErrorString());
        const QString requestString = QString::fromUtf8(request->getRequestString());
        QString logMsg(QString::fromUtf8("Error performing device center %1 request: \"%2\"")
                           .arg(requestString, errorMsg));
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
    }
}

void DeviceCentre::onSyncStateChanged(mega::MegaApi*, mega::MegaSync* sync)
{
    const QmlSyncData syncObject(sync);
    updateLocalData(syncObject);
}

void DeviceCentre::onSyncStatsUpdated(mega::MegaApi*, mega::MegaSyncStats* syncStats)
{
    const QmlSyncData syncObject(syncStats);
    updateLocalData(syncObject);
}

QUrl DeviceCentre::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/deviceCentre/DeviceCentreDialog.qml"));
}

QString DeviceCentre::contextName()
{
    return QString::fromUtf8("deviceCentreAccess");
}

void DeviceCentre::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("DeviceCentre", 1, 0);
        qmlRegisterType<SyncModel>("SyncModel", 1, 0, "SyncModel");
        qmlRegisterType<DeviceModel>("DeviceModel", 1, 0, "DeviceModel");
        qmlRegisterUncreatableMetaObject(
            QmlSyncType::staticMetaObject,
            "QmlSyncType",
            1,
            0,
            "QmlSyncType",
            QString::fromUtf8("QmlSyncType is not meant to be created"));
        qmlRegisterUncreatableMetaObject(DeviceOs::staticMetaObject,
                                         "DeviceOs",
                                         1,
                                         0,
                                         "DeviceOs",
                                         QString::fromUtf8("DeviceOs is not meant to be created"));
        qmlRegisterUncreatableMetaObject(
            SyncStatus::staticMetaObject,
            "SyncStatus",
            1,
            0,
            "SyncStatus",
            QString::fromUtf8("SyncStatus is not meant to be created"));
        qRegisterMetaType<DeviceData>();
        qmlRegistrationDone = true;
    }
}

void DeviceCentre::openAddBackupDialog()
{
    const bool comesFromSettings = true;
    CreateRemoveBackupsManager::addBackup(comesFromSettings);
}

QString DeviceCentre::getCurrentDeviceId()
{
    std::unique_ptr<const char> rawDeviceId{mMegaApi->getDeviceId()};
    return QString::fromLatin1(rawDeviceId.get());
}

void DeviceCentre::retrieveDeviceData(const QString& deviceId)
{
    mDeviceIdFromLastRequest = deviceId;
    mMegaApi->getDeviceName(deviceId.toLatin1().constData());
    mMegaApi->getBackupInfo();
}

QString DeviceCentre::getSizeString(long long bytes) const
{
    return Utilities::getSizeString(bytes);
}

void DeviceCentre::updateLocalData(const mega::MegaBackupInfoList& backupList)
{
    // TODO Keep a local model with all the data, including other devices.
    // We will need other devices data in the future.
    const BackupList deviceBackupList =
        filterBackupList(mDeviceIdFromLastRequest.toLatin1().constData(), backupList);

    mCachedDeviceData.os = DeviceOs::getCurrentOS();
    for (const auto& backup: deviceBackupList)
    {
        QmlSyncData newSync(backup, mMegaApi);
        mSyncModel->addOrUpdate(newSync);
    }
    updateDeviceData();
}

void DeviceCentre::updateLocalData(const QmlSyncData& syncObj)
{
    mSyncModel->addOrUpdate(syncObj);
    updateDeviceData();
    emit deviceDataUpdated();
}

void DeviceCentre::updateDeviceData()
{
    mCachedDeviceData.folderCount = mSyncModel->rowCount();
    mCachedDeviceData.status = mSyncModel->computeDeviceStatus();
    mCachedDeviceData.totalSize = mSyncModel->computeTotalSize();
}

DeviceCentre::BackupList DeviceCentre::filterBackupList(const char* deviceId,
                                                        const mega::MegaBackupInfoList& backupList)
{
    BackupList filteredList;
    const auto numBackups = backupList.size();
    for (uint backupIndex = 0; backupIndex < numBackups; backupIndex++)
    {
        auto backupInfo = backupList.get(backupIndex);
        if (strcmp(backupInfo->deviceId(), deviceId) == 0)
        {
            filteredList.push_back(backupInfo);
        }
    }
    return filteredList;
}

DeviceOs::Os DeviceCentre::getCurrentOS()
{
    return mCachedDeviceData.os;
}

void DeviceCentre::openAddSyncDialog()
{
    const bool comesFromSettings = true;
    CreateRemoveSyncsManager::addSync(mega::INVALID_HANDLE, comesFromSettings);
}

DeviceModel* DeviceCentre::getDeviceModel() const
{
    return mDeviceModel;
}

SyncModel* DeviceCentre::getSyncModel() const
{
    return mSyncModel;
}
