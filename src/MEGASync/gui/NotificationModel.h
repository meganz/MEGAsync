#ifndef QNOTIFICATIONSMODEL_H
#define QNOTIFICATIONSMODEL_H

#include "NotificationItem.h"

#include <QAbstractItemModel>
#include <QCache>

struct NotifTest
{
    int64_t id = 0;
    std::string title = "";
    std::string description = "";
    std::string imageName = "";
    std::string iconName = "";
    std::string imagePath = "";
    int64_t start = 0;
    int64_t end = 0;
    bool showBanner = false;
    std::map<std::string, std::string> callToAction1{};
    std::map<std::string, std::string> callToAction2{};
};

class NotificationModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit NotificationModel(QObject* parent = 0);
    virtual ~NotificationModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QCache<int, NotificationItem> notificationItems;

private:
    QMap<int, NotifTest*> mNotifMap;

};

#endif // QNOTIFICATIONSMODEL_H
