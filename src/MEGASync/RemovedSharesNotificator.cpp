#include "RemovedSharesNotificator.h"
#include "megaapi.h"
#include "mega/types.h"

constexpr auto alertClusterMaxElapsedTime = std::chrono::minutes(10);

void RemovedSharesNotificator::removeObsoleteAlertClusters()
{
    for(const auto& clusterTimestamp : mClusterTimestamps)
    {
        const auto elapsedTime = std::chrono::system_clock::now() - clusterTimestamp.second;
        if(elapsedTime > alertClusterMaxElapsedTime)
        {
            const auto alertClusterIterator = mAlertClusters.find(clusterTimestamp.first);
            const auto itemFound(alertClusterIterator != mAlertClusters.end());
            if(itemFound)
            {
                mAlertClusters.erase(clusterTimestamp.first);
            }
        }
    }
}

void RemovedSharesNotificator::addUserAlert(mega::MegaUserAlert *userAlert, const QString& userName)
{
    removeObsoleteAlertClusters();

    const auto userAlertId = userAlert->getId();
    const auto alertClusterIterator = mAlertClusters.find(userAlertId);
    const auto alertClusterFound = alertClusterIterator != mAlertClusters.end();
    if(!alertClusterFound)
    {
        auto userAlertTimedClustering = ::mega::make_unique<UserAlertTimedClustering>();
        QObject::connect(userAlertTimedClustering.get(), &UserAlertTimedClustering::sendUserAlert,
                         this, &RemovedSharesNotificator::sendClusteredAlert);
        mAlertClusters[userAlert->getId()] = std::move(userAlertTimedClustering);
    }
    mAlertClusters[userAlertId]->addUserAlert(userAlert, userName);
    mClusterTimestamps[userAlertId] = std::chrono::system_clock::now();
}
