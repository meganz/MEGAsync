#ifndef USER_MESSAGES_CONTROLLER_H
#define USER_MESSAGES_CONTROLLER_H

#include "UserMessageTypes.h"

#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "QTMegaGlobalListener.h"

#include <QAbstractItemModel>

class UserMessageModel;
class UserMessageProxyModel;

class UserMessageController : public QObject, public mega::MegaRequestListener, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    explicit UserMessageController(QObject* parent = nullptr);
    virtual ~UserMessageController() = default;

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;
    void onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list) override;

    void reset();
    bool hasNotifications();
    bool hasElementsOfType(MessageType type);
    void applyFilter(MessageType type);
    void requestNotifications() const;
    void ackSeenUserMessages();
    QAbstractItemModel* getModel() const;

signals:
    void userAlertsUpdated(mega::MegaUserAlertList* list);
    void unseenAlertsChanged(const UnseenUserMessagesMap& alerts);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    std::unique_ptr<UserMessageModel> mUserMessagesModel;
    std::unique_ptr<UserMessageProxyModel> mUserMessagesProxyModel;
    long long mAllUnseenAlerts;

    void populateUserAlerts(mega::MegaUserAlertList* alertList);
    void checkUseenNotifications();

};

#endif // USER_MESSAGES_CONTROLLER_H
