#include "DeviceCentre.h"

#include "control/TextDecorator.h"
#include "CreateRemoveBackupsManager.h"
#include "CreateRemoveSyncsManager.h"
#include "DialogOpener.h"
#include "MegaApplication.h"
#include "QmlDialogWrapper.h"
#include "QmlUtils.h"
#include "ServiceUrls.h"
#include "StalledIssuesModel.h"
#include "SyncController.h"
#include "SyncExclusions.h"
#include "SyncInfo.h"
#include "syncs/control/MegaIgnoreManager.h"

#include <set>

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
    connect(mSyncModel, &QAbstractItemModel::rowsInserted, this, &DeviceCentre::rowCountChanged);
    connect(mSyncModel, &QAbstractItemModel::rowsRemoved, this, &DeviceCentre::rowCountChanged);
    connect(mSyncModel, &QAbstractItemModel::modelReset, this, &DeviceCentre::rowCountChanged);

    mDelegateListener = std::make_unique<mega::QTMegaListener>(mMegaApi, this);
    mMegaApi->addListener(mDelegateListener.get());

    mSizeInfoTimer.setInterval(DELAY_TO_NEXT_CALL_IN_MS);
    mSizeInfoTimer.setSingleShot(true);
    connect(&mSizeInfoTimer,
            &QTimer::timeout,
            this,
            [this]()
            {
                mMegaApi->getBackupInfo();
            });
    SyncInfo::instance()->dismissUnattendedDisabledSyncs(
        {mega::MegaSync::SyncType::TYPE_BACKUP, mega::MegaSync::SyncType::TYPE_TWOWAY});
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
        // Receiving device name
        if (request->getParamType() == mega::MegaApi::USER_ATTR_DEVICE_NAMES)
        {
            // if current device name add it to the model and reset it
            if (QString::fromUtf8(request->getText()) ==
                QmlUtils::getQmlInstance(QmlManager::instance()->getEngine(), nullptr)
                    ->getCurrentDeviceID())
            {
                mCachedDeviceData.name = QString::fromUtf8(request->getName());
                mDeviceModel->reset(mDeviceIdFromLastRequest, mCachedDeviceData);
                emit deviceDataUpdated();
            }
            // Add alldevice name to device model
            mDeviceModel->addDeviceName(QString::fromUtf8(request->getText()),
                                        QString::fromUtf8(request->getName()));
        }
        else if (request->getType() == mega::MegaRequest::TYPE_BACKUP_INFO)
        {
            mega::MegaBackupInfoList* backupList = request->getMegaBackupInfoList();
            requestDeviceNames(*backupList);
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
        else if (request->getType() == mega::MegaRequest::TYPE_BACKUP_REMOVE)
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
    if (syncStats->isScanning())
    {
        const QmlSyncData syncObject(syncStats);
        updateLocalData(syncObject);
    }
    else
    {
        mMegaApi->getBackupInfo();
    }
}

void DeviceCentre::onSyncDeleted(mega::MegaApi* api, mega::MegaSync* sync)
{
    mSyncModel->remove(sync->getBackupId());
    updateDeviceData();
    emit deviceDataUpdated();
}

QUrl DeviceCentre::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/deviceCentre/DeviceCentreDialog.qml"));
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
    auto activeSyncs = SyncInfo::instance()->getAllSyncSettings();
    BackupList filteredList;
    const auto numBackups = backupList.size();
    for (uint backupIndex = 0; backupIndex < numBackups; backupIndex++)
    {
        auto backupInfo = backupList.get(backupIndex);
        if (strcmp(backupInfo->deviceId(), deviceId) == 0)
        {
            bool isSyncActive = std::find_if(std::begin(activeSyncs),
                                             std::end(activeSyncs),
                                             [&backupInfo](std::shared_ptr<SyncSettings> sync)
                                             {
                                                 return sync->backupId() == backupInfo->id();
                                             }) != std::end(activeSyncs);

            if (isSyncActive)
            {
                filteredList.push_back(backupInfo);
            }
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
    CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::MAIN_APP_ORIGIN, mega::INVALID_HANDLE);
}

DeviceModel* DeviceCentre::getDeviceModel() const
{
    return mDeviceModel;
}

SyncModel* DeviceCentre::getSyncModel() const
{
    return mSyncModel;
}

void DeviceCentre::renameCurrentDevice(const QString& newName)
{
    mMegaApi->setDeviceName(mDeviceIdFromLastRequest.toStdString().c_str(),
                            newName.toStdString().c_str());
}

void DeviceCentre::openInMega(int row) const
{
    const auto handle = mSyncModel->getHandle(row);
    if (handle)
    {
        Utilities::openInMega(*handle);
    }
}

void DeviceCentre::showInFolder(int row) const
{
    const auto localFolder = mSyncModel->getLocalFolder(row);
    if (!localFolder.isEmpty())
    {
        Utilities::openUrl(QUrl::fromLocalFile(localFolder));
    }
}

void DeviceCentre::pauseSync(int row) const
{
    changeSyncStatus(row,
                     [](std::shared_ptr<SyncSettings> settings)
                     {
                         SyncController::instance().setSyncToPause(settings);
                     });
}

void DeviceCentre::resumeSync(int row) const
{
    changeSyncStatus(row,
                     [](std::shared_ptr<SyncSettings> settings)
                     {
                         SyncController::instance().setSyncToRun(settings);
                     });
}

void DeviceCentre::rescanSync(int row, bool deepRescan) const
{
    const auto syncID = mSyncModel->getSyncID(row);
    if (syncID)
    {
        mMegaApi->rescanSync(*syncID, deepRescan);
    }
    else
    {
        mega::MegaApi::log(
            mega::MegaApi::LOG_LEVEL_WARNING,
            QString::fromUtf8("Attempted rescanSync with invalid syncID").toUtf8().constData());
    }
}

void DeviceCentre::manageExclusions(int row) const
{
    const auto folderPath = mSyncModel->getLocalFolder(row);
    QFileInfo syncDir(folderPath);
    if (syncDir.exists())
    {
        QPointer<QmlDialogWrapper<SyncExclusions>> exclusions =
            new QmlDialogWrapper<SyncExclusions>(nullptr, folderPath);
        DialogOpener::showDialog(exclusions);
    }
}

void DeviceCentre::stopSync(int row) const
{
    const auto syncType = mSyncModel->getType(row);
    const auto syncID = mSyncModel->getSyncID(row);
    if (!syncID)
    {
        mega::MegaApi::log(
            mega::MegaApi::LOG_LEVEL_WARNING,
            QString::fromUtf8("Attempted stopSync with invalid syncID").toUtf8().constData());
        return;
    }
    const auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(*syncID);
    if (syncType == QmlSyncType::Type::BACKUP)
    {
        CreateRemoveBackupsManager::removeBackup(syncSettings, nullptr);
    }
    else
    {
        const auto handle = mSyncModel->getHandle(row);
        if (handle && *handle != mega::INVALID_HANDLE &&
            !CreateRemoveSyncsManager::removeSync(*handle, nullptr))
        {
            mMegaApi->removeBackup(*syncID);
        }
    }
}

void DeviceCentre::requestDeviceNames(const mega::MegaBackupInfoList& backupList) const
{
    std::set<QString> requestedNames;
    const auto numBackups = backupList.size();
    for (uint backupIndex = 0; backupIndex < numBackups; backupIndex++)
    {
        auto backupInfo = backupList.get(backupIndex);
        if (requestedNames.find(QString::fromUtf8(backupInfo->deviceId())) == requestedNames.end())
        {
            mMegaApi->getDeviceName(backupInfo->deviceId());
            requestedNames.insert(QString::fromUtf8(backupInfo->deviceId()));
        }
    }
}

void DeviceCentre::rebootSync(int row) const
{
    const auto syncID = mSyncModel->getSyncID(row);
    if (!syncID)
    {
        return;
    }
    const auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(*syncID);
    if (!syncSettings)
    {
        return;
    }
    SyncController::instance().resetSync(syncSettings,
                                         mega::MegaSync::SyncRunningState::RUNSTATE_DISABLED);
}

void DeviceCentre::changeSyncStatus(int row,
                                    std::function<void(std::shared_ptr<SyncSettings>)> action) const
{
    const auto syncID = mSyncModel->getSyncID(row);
    if (!syncID)
    {
        return;
    }

    const auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(*syncID);
    action(syncSettings);
}

bool DeviceCentre::deviceNameAlreadyExists(const QString& name) const
{
    return mDeviceModel->deviceNameAlreadyExists(name);
}

void DeviceCentre::learnMore() const
{
    Utilities::openUrl(ServiceUrls::getContactSupportUrl());
}

void DeviceCentre::applyPreviousExclusionRules() const
{
    MessageDialogInfo msgInfo;
    msgInfo.titleText = /*tr*/ QString::fromUtf8("Apply previous exclusion rules?");
    Text::Bold boldDecorator;
    boldDecorator.process(msgInfo.titleText);
    msgInfo.descriptionText =
        /*tr*/ QString::fromUtf8(
            "The exclusion rules you set up in a previous version of the app will be applied to "
            "all "
            "of your syncs and backups. Any rules created since then will be overwritten.");
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok, /*tr*/ QString::fromUtf8("Apply"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.finishFunc = [](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Ok)
        {
            // Step 0: Remove old default ignore
            const auto defaultIgnoreFolder = Preferences::instance()->getDataPath();
            const auto defaultIgnorePath =
                defaultIgnoreFolder + QString::fromUtf8("/") +
                QString::fromUtf8(MegaIgnoreManager::MEGA_IGNORE_DEFAULT_FILE_NAME);
            QFile::remove(defaultIgnorePath);

            // Step 1: Replace default ignore with one populated with legacy rules
            MegaSyncApp->getMegaApi()->exportLegacyExclusionRules(
                defaultIgnoreFolder.toStdString().c_str());
            QFile::rename(defaultIgnoreFolder + QString::fromUtf8("/") +
                              QString::fromUtf8(MegaIgnoreManager::MEGA_IGNORE_FILE_NAME),
                          defaultIgnorePath);

            // Step 2: Replace existing mega ignores files in all syncs
            const auto syncsSettings = SyncInfo::instance()->getAllSyncSettings();
            for (auto& sync: syncsSettings)
            {
                QFile ignoreFile(sync->getLocalFolder() + QString::fromUtf8("/") +
                                 QString::fromUtf8(MegaIgnoreManager::MEGA_IGNORE_FILE_NAME));
                if (ignoreFile.exists())
                {
                    ignoreFile.moveToTrash();
                }
                MegaSyncApp->getMegaApi()->exportLegacyExclusionRules(
                    sync->getLocalFolder().toStdString().c_str());
            }
        }
    };
    MessageDialogOpener::warning(msgInfo);
}

void DeviceCentre::onSmartModeSelected() const
{
    Preferences::instance()->setStalledIssuesMode(Preferences::StalledIssuesModeType::Smart);
    // Update the model to fix automatically the issues
    MegaSyncApp->getStalledIssuesModel()->updateActiveStalledIssues();
}

void DeviceCentre::onAdvancedModeSelected() const
{
    Preferences::instance()->setStalledIssuesMode(Preferences::StalledIssuesModeType::Advance);
}

bool DeviceCentre::isSmartModeSelected() const
{
    return Preferences::instance()->isStalledIssueSmartModeActivated();
}

void DeviceCentre::showWarningMessageDialog(const QString& descriptionText) const
{
    MessageDialogInfo msgInfo;
    msgInfo.textFormat = Qt::TextFormat::RichText;
    msgInfo.descriptionText = descriptionText;
    MessageDialogOpener::warning(msgInfo);
}

void DeviceCentre::showRebootWarningDialog(const QString& titleText,
                                           const QString& descriptionText,
                                           int row) const
{
    MessageDialogInfo msgInfo;
    msgInfo.textFormat = Qt::TextFormat::RichText;
    msgInfo.titleText = titleText;
    msgInfo.descriptionText = descriptionText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok,
                         /*QCoreApplication::translate ("Strings",*/ QString::fromUtf8("Continue"));
    textsByButton.insert(QMessageBox::Cancel,
                         /*QCoreApplication::translate ("Strings",*/ QString::fromUtf8("Cancel"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.defaultButton = QMessageBox::Ok;
    msgInfo.finishFunc = [this, row](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Ok)
        {
            rebootSync(row);
        }
    };
    MessageDialogOpener::warning(msgInfo);
}

int DeviceCentre::getRowCount() const
{
    return mSyncModel->rowCount();
}
