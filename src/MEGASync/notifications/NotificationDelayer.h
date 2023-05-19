#pragma once
#include "UserAlertTimedClustering.h"
#include <queue>
#include <memory>
#include <QObject>

namespace mega {
class MegaUserAlert;
}

class NotificationDelayer: public QObject
{
    Q_OBJECT
public:
    void addUserAlert(mega::MegaUserAlert* userAlert, const QString &userName);

signals:
    void sendClusteredAlert(mega::MegaUserAlert* alert, uint64_t number);

private:
    using AlertId = unsigned;
    std::map<AlertId, std::unique_ptr<UserAlertTimedClustering>> mAlertClusters;
    std::map<AlertId, std::chrono::system_clock::time_point> mClusterTimestamps;

    void removeObsoleteAlertClusters();
};
