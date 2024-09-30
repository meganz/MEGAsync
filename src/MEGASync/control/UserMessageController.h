#ifndef USER_MESSAGE_CONTROLLER_H
#define USER_MESSAGE_CONTROLLER_H

#include "UserMessageTypes.h"
#include "UserMessageModel.h"
#include "UserMessageProxyModel.h"

#include "megaapi.h"
#include "QTMegaGlobalListener.h"

#include <QAbstractItemModel>

#include <memory>

namespace mega
{
class QTMegaRequestListener;
}

class UserMessageController : public QObject, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    explicit UserMessageController(QObject* parent = nullptr);
    virtual ~UserMessageController() = default;

    void onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list) override;

    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

    bool hasNotifications();
    bool hasElementsOfType(MessageType type);
    void applyFilter(MessageType type);
    void requestNotifications() const;
    void ackSeenUserMessages();
    QAbstractItemModel* getModel() const;

signals:
    void userAlertsUpdated(mega::MegaUserAlertList* list);
    void unseenAlertsChanged(const UnseenUserMessagesMap& alerts);
    void userMessagesReceived();

protected:
    bool event(QEvent* event) override;

private:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    std::unique_ptr<UserMessageModel> mUserMessagesModel;
    std::unique_ptr<UserMessageProxyModel> mUserMessagesProxyModel;
    long long mAllUnseenAlerts;

    void populateUserAlerts(mega::MegaUserAlertList* alertList);
    void checkUseenNotifications();

};

#endif // USER_MESSAGE_CONTROLLER_H
