#include "DeviceCenter.h"

#include "CreateRemoveBackupsManager.h"
#include "CreateRemoveSyncsManager.h"
#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

DeviceCenter::DeviceCenter(QObject* parent):
    QMLComponent(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSyncModel(new SyncModel(this)),
    mDeviceModel(new DeviceModel(this))
{
    registerQmlModules();

    mDelegateListener = new mega::QTMegaListener(mMegaApi, this);
    mMegaApi->addListener(mDelegateListener);

    const int delayToNextCallInMs = 1000;
    mSizeInfoTimer.setInterval(delayToNextCallInMs);
    mSizeInfoTimer.setSingleShot(true);
    connect(&mSizeInfoTimer, &QTimer::timeout, [this]() {
        mMegaApi->getBackupInfo();
    });
}

DeviceCenter::~DeviceCenter()
{
    mMegaApi->removeListener(mDelegateListener);
    mSizeInfoTimer.stop();
}

void DeviceCenter::onRequestFinish(mega::MegaApi* api,
                                   mega::MegaRequest* request,
                                   mega::MegaError* e)
{
    if (request->getParamType() == mega::MegaApi::USER_ATTR_DEVICE_NAMES)
    {
        mCachedDeviceData.name = QString::fromUtf8(request->getName());
        mDeviceModel->reset(mDeviceIdFromLastRequest, mCachedDeviceData);
        emit deviceDataUpdated(mCachedDeviceData);
    }
    else if (request->getType() == mega::MegaRequest::TYPE_BACKUP_INFO)
    {
        mega::MegaBackupInfoList* backupList = request->getMegaBackupInfoList();
        updateLocalData(*backupList);

        if (mSyncModel->hasUpdatingStatus())
        {
            mSizeInfoTimer.start();
        }
        emit deviceDataUpdated(mCachedDeviceData);
    }
    else if (request->getType() == mega::MegaRequest::TYPE_ADD_SYNC)
    {
        const QmlSyncData syncObject(request, mMegaApi);
        mSyncModel->addOrUpdate(syncObject);

        mMegaApi->getBackupInfo();
        emit deviceDataUpdated(mCachedDeviceData);
    }
    else if (request->getType() == mega::MegaRequest::TYPE_REMOVE_SYNC)
    {
        mSyncModel->remove(request->getParentHandle());
        updateDeviceData();
        emit deviceDataUpdated(mCachedDeviceData);
    }
}

void DeviceCenter::onSyncStateChanged(mega::MegaApi*, mega::MegaSync* sync)
{
    const QmlSyncData syncObject(sync);
    updateLocalData(syncObject);
}

void DeviceCenter::onSyncStatsUpdated(mega::MegaApi*, mega::MegaSyncStats* syncStats)
{
    const QmlSyncData syncObject(syncStats);
    updateLocalData(syncObject);
}

QUrl DeviceCenter::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/deviceCenter/DeviceCenterDialog.qml"));
}

QString DeviceCenter::contextName()
{
    return QString::fromUtf8("deviceCenterAccess");
}

void DeviceCenter::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("DeviceCenter", 1, 0);
        qmlRegisterType<QmlDialog>("DeviceCenterQmlDialog", 1, 0, "DeviceCenterQmlDialog");
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

void DeviceCenter::openAddBackupDialog()
{
    const bool comesFromSettings = true;
    CreateRemoveBackupsManager::addBackup(comesFromSettings);
}

QString DeviceCenter::getThisDeviceId()
{
    auto megaApi = MegaSyncApp->getMegaApi();
    const char* rawDeviceId = megaApi->getDeviceId();
    const QString deviceId = QString::fromLatin1(rawDeviceId);
    delete rawDeviceId;
    return deviceId;
}

void DeviceCenter::retrieveDeviceData(const QString& deviceId)
{
    mDeviceIdFromLastRequest = deviceId;
    mMegaApi->getDeviceName(deviceId.toLatin1().constData());
    mMegaApi->getBackupInfo();
}

QString DeviceCenter::getSizeString(unsigned long long bytes)
{
    return Utilities::getSizeString(bytes);
}

void DeviceCenter::updateLocalData(mega::MegaBackupInfoList& backupList)
{
    // TODO Keep a local model with all the data, including other devices.
    // We will need other devices data in the future.
    const BackupList deviceBackupList =
        filterBackupList(mDeviceIdFromLastRequest.toLatin1().constData(), backupList);

    mCachedDeviceData.os = DeviceOs::getCurrentOS();
    for (const auto& backup: qAsConst(deviceBackupList))
    {
        QmlSyncData newSync(backup, mMegaApi);
        mSyncModel->addOrUpdate(newSync);
    }
    updateDeviceData();
}

void DeviceCenter::updateLocalData(const QmlSyncData& syncObj)
{
    mSyncModel->addOrUpdate(syncObj);
    updateDeviceData();
    emit deviceDataUpdated(mCachedDeviceData);
}

void DeviceCenter::updateDeviceData()
{
    mCachedDeviceData.folderCount = mSyncModel->rowCount();
    mCachedDeviceData.status = mSyncModel->computeDeviceStatus();
    mCachedDeviceData.totalSize = mSyncModel->computeTotalSize();
}

DeviceCenter::BackupList DeviceCenter::filterBackupList(const char* deviceId,
                                                        mega::MegaBackupInfoList& backupList)
{
    BackupList filteredList;
    const int numBackups = backupList.size();
    for (int i = 0; i < numBackups; i++)
    {
        auto backupInfo = backupList.get(i);
        if (strcmp(backupInfo->deviceId(), deviceId) == 0)
        {
            filteredList.push_back(backupInfo);
        }
    }
    return filteredList;
}

DeviceOs::Os DeviceCenter::getCurrentOS()
{
    return DeviceOs::getCurrentOS();
}

void DeviceCenter::openAddSyncDialog()
{
    const bool comesFromSettings = true;
    CreateRemoveSyncsManager::addSync(mega::INVALID_HANDLE, comesFromSettings);
}
