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

void UserAlertTimedClustering::addUserAlert(mega::MegaUserAlert *alert)
{
    std::lock_guard<std::mutex> lock(mUserAlertMutex);
    if(mUserAlert)
    {
        delete mUserAlert;
    }
    mUserAlert = alert->copy();
    if(!mClusteringTimer.isActive())
    {
        mClusteringTimer.start(clusterMaxTime);
    }
}

QString getRemovedItemsMessage(int64_t removedItems, const QString& email)
{
    if (removedItems == 1)
    {
        return QCoreApplication::translate("OsNotifications", "[A] removed 1 item")
                .replace(QString::fromUtf8("[A]"), email);
    }
    else
    {
         return QCoreApplication::translate("OsNotifications", "[A] removed [B] items")
                 .replace(QString::fromUtf8("[A]"), email)
                 .replace(QString::fromUtf8("[B]"), QString::number(removedItems));
    }
}

void UserAlertTimedClustering::onClusterTimerTimeout()
{
    std::lock_guard<std::mutex> lock(mUserAlertMutex);
    const auto totalRemovedItems{mUserAlert->getNumber(0)};
    const auto currentRemovedItems{totalRemovedItems - mPreviousTotalRemovedItems};
    mPreviousTotalRemovedItems = totalRemovedItems;
    const auto email{QString::fromUtf8(mUserAlert->getEmail())};
    const auto message{getRemovedItemsMessage(currentRemovedItems, email)};
    emit sendUserAlert(mUserAlert, message);
}
