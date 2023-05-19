#include "UserAlertTimedClustering.h"
#include "megaapi.h"
#include <QCoreApplication>

constexpr int clusterMaxTime{5000};//5 seconds cluster time

UserAlertTimedClustering::UserAlertTimedClustering()
    :mUserAlert{nullptr}, mPreviousTotalRemovedItems{0}
{
    QObject::connect(&mClusteringTimer, &QTimer::timeout, this, &UserAlertTimedClustering::onClusterTimerTimeout);
    mClusteringTimer.setSingleShot(true);
}

UserAlertTimedClustering::~UserAlertTimedClustering()
{
    if(mUserAlert)
    {
        delete mUserAlert;
    }
}

void UserAlertTimedClustering::addUserAlert(mega::MegaUserAlert *alert, const QString& userName)
{
    std::lock_guard<std::mutex> lock(mUserAlertMutex);
    if(mUserAlert)
    {
        delete mUserAlert;
    }
    mUserAlert = alert->copy();
    mUserName = userName;
    if(!mClusteringTimer.isActive())
    {
        mClusteringTimer.start(clusterMaxTime);
    }
}

QString getRemovedItemsMessage(int64_t removedItems, const QString& userName)
{
    const int removedItemsAsInt = static_cast<int>(removedItems);
    return QCoreApplication::translate("OsNotifications", "[A] removed %n item", "", removedItemsAsInt)
            .replace(QString::fromUtf8("[A]"), userName);
}

void UserAlertTimedClustering::onClusterTimerTimeout()
{
    std::lock_guard<std::mutex> lock(mUserAlertMutex);
    const auto totalRemovedItems = mUserAlert->getNumber(0);
    const auto currentRemovedItems = totalRemovedItems - mPreviousTotalRemovedItems;
    mPreviousTotalRemovedItems = totalRemovedItems;
    const QString message{getRemovedItemsMessage(currentRemovedItems, mUserName)};
    emit sendUserAlert(mUserAlert, currentRemovedItems);
}
