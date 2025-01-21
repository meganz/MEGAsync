#ifndef NOTIFICATION_DELAYED_H
#define NOTIFICATION_DELAYED_H

#include "UserAlertTimedClustering.h"

#include <QObject>

#include <memory>

namespace mega {
class MegaUserAlert;
}

class NotificationDelayer: public QObject
{
    Q_OBJECT
public:
    void addUserAlert(mega::MegaUserAlert* userAlert, const QString &userName);

signals:
    void sendClusteredAlert(mega::MegaUserAlert* alert, const QString& message);

private:
    using AlertId = unsigned;
    std::map<AlertId, std::unique_ptr<UserAlertTimedClustering>> mAlertClusters;
    std::map<AlertId, std::chrono::system_clock::time_point> mClusterTimestamps;

    void removeObsoleteAlertClusters();
};
#endif
