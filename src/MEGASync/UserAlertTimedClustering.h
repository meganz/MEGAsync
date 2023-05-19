#pragma once
#include <mutex>
#include <QTimer>

namespace mega {
class MegaUserAlert;
}

class UserAlertTimedClustering: public QObject
{
    Q_OBJECT
public:
    UserAlertTimedClustering();
    ~UserAlertTimedClustering();
    void addUserAlert(mega::MegaUserAlert* alert, const QString &userName);

private:
    mega::MegaUserAlert* mUserAlert;
    QString mUserName;
    std::mutex mUserAlertMutex;
    QTimer mClusteringTimer;
    using TotalRemovedItems = int64_t;
    TotalRemovedItems mPreviousTotalRemovedItems;

private slots:
    void onClusterTimerTimeout();

signals:
    void sendUserAlert(mega::MegaUserAlert* userAlert, int64_t number);
};
