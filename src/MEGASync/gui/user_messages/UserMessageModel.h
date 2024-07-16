#ifndef USER_MESSAGE_MODEL_H
#define USER_MESSAGE_MODEL_H

#include "UserMessageTypes.h"

#include <QAbstractItemModel>

namespace mega
{
class MegaUserAlert;
class MegaUserAlertList;
class MegaNotificationList;
class MegaNotification;
}

class UserMessage;

class UserMessageModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    using QAbstractItemModel::QAbstractItemModel;
    virtual ~UserMessageModel() = default;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void processAlerts(mega::MegaUserAlertList* alerts);
    void processNotifications(const mega::MegaNotificationList* notifications);
    bool hasAlertsOfType(MessageType type);
    UnseenUserMessagesMap getUnseenNotifications() const;
    uint32_t checkLocalLastSeenNotification();
    void setLastSeenNotification(uint32_t id);

private:
    class SeenStatusManager
    {

    public:
        SeenStatusManager() = default;
        virtual ~SeenStatusManager() = default;

        void markAsUnseen(MessageType type);
        bool markNotificationAsUnseen(uint32_t id);
        void markAsSeen(MessageType type);
        UnseenUserMessagesMap getUnseenUserMessages() const;

        void setLastSeenNotification(uint32_t id);
        uint32_t getLastSeenNotification() const;
        void setLocalLastSeenNotification(uint32_t id);
        uint32_t getLocalLastSeenNotification() const;

    private:
        UnseenUserMessagesMap mUnseenNotifications;
        uint32_t mLastSeenNotification = 0;
        uint32_t mLocalLastSeenNotification = 0;

    };

    QList<UserMessage*> mNotifications;
    SeenStatusManager mSeenStatusManager;

    void insertAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void updateAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void removeAlerts(const QList<mega::MegaUserAlert*>& alerts);

    void insertNotifications(const mega::MegaNotificationList* notifications);
    void removeNotifications(const mega::MegaNotificationList* notifications);

    auto findAlertById(unsigned id);
    auto findNotificationById(int64_t id);

};

#endif // USER_MESSAGE_MODEL_H
