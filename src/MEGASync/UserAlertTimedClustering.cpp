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

QString getMessage(int64_t itemCount, const QString& userName, int type)
{
    const int itemCountAsInt = static_cast<int>(itemCount);

    QString message;
    switch(type)
    {
    case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
    {
        message = QCoreApplication::translate("OsNotifications", "[A] removed %n item", "", itemCountAsInt)
                      .replace(QString::fromUtf8("[A]"), userName);
        break;
    }
    case mega::MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
    {
        message = QCoreApplication::translate("DesktopNotifications", "[A] updated %n item", "", itemCountAsInt)
                      .replace(QString::fromUtf8("[A]"), userName);
        break;
    }
    }
    return message;
}

void UserAlertTimedClustering::onClusterTimerTimeout()
{
    std::lock_guard<std::mutex> lock(mUserAlertMutex);
    const auto totalRemovedItems = mUserAlert->getNumber(0);
    const auto currentRemovedItems = totalRemovedItems - mPreviousTotalRemovedItems;
    mPreviousTotalRemovedItems = totalRemovedItems;
    emit sendUserAlert(mUserAlert, getMessage(currentRemovedItems, mUserName, mUserAlert->getType()));
}
