#include "DeviceNameChecker.h"

#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "Utilities.h"

const static int INITIAL_PENDING_REQUEST = 2;

DeviceNameChecker::DeviceNameChecker(QObject* parent, QString deviceNameCandidate):
    QThread(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mDeviceNameCandidate(deviceNameCandidate)
{}

void DeviceNameChecker::run()
{
    mMyBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    mDeviceName = UserAttributes::DeviceNames::requestDeviceName();

    mPendingRequests = INITIAL_PENDING_REQUEST;
    mBackupDeviceNames.clear();

    if (mDeviceName->isAttributeReady())
    {
        updateReadyCondition();
    }
    else
    {
        connect(mDeviceName.get(),
                &UserAttributes::DeviceNames::attributeReady,
                this,
                &DeviceNameChecker::updateReadyCondition);
    }

    if (mMyBackupsHandle->isAttributeReady())
    {
        fetchBackupDeviceNames();
    }
    else
    {
        connect(mMyBackupsHandle.get(),
                &UserAttributes::MyBackupsHandle::attributeReady,
                this,
                &DeviceNameChecker::fetchBackupDeviceNames);
    }

    if (mPendingRequests != 0)
    {
        exec();
    }
}

void DeviceNameChecker::fetchBackupDeviceNames()
{
    if (mMyBackupsHandle->isAttributeReady())
    {
        auto backupHandle = mMyBackupsHandle->getMyBackupsHandle();
        if (backupHandle != mega::INVALID_HANDLE)
        {
            std::unique_ptr<mega::MegaNode> backupFolder(mMegaApi->getNodeByHandle(backupHandle));

            std::unique_ptr<mega::MegaNodeList> nodeList(mMegaApi->getChildren(backupFolder.get()));

            for (int nodeIndex = 0; nodeIndex < nodeList->size(); ++nodeIndex)
            {
                auto node = nodeList->get(nodeIndex);
                if (node != nullptr && node->getType() == mega::MegaNode::TYPE_FOLDER)
                {
                    mBackupDeviceNames.push_back(QString::fromUtf8(node->getName()));
                }
            }
        }
    }

    updateReadyCondition();
}

void DeviceNameChecker::updateReadyCondition()
{
    --mPendingRequests;
    if (mPendingRequests == 0)
    {
        emit deviceNameCheck(checkDeviceName(mDeviceNameCandidate));
    }
}

bool DeviceNameChecker::checkDeviceName(QString deviceName)
{
    QString currentDeviceName = mDeviceName->getDeviceName();
    if (currentDeviceName == deviceName)
    {
        /* it's ok, we try to set as a device name our current device name */
        return true;
    }

    auto deviceNameMap = mDeviceName->getDeviceNames();
    auto deviceNames = deviceNameMap.values();
    if (deviceNames.contains(deviceName))
    {
        /* error, device name used in another device */
        return false;
    }

    /* the device name is used in previous backup folders ?? */
    return !mBackupDeviceNames.contains(deviceName);
}
