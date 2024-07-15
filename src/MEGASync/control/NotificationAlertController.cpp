#include "NotificationAlertController.h"

#include "UserMessageModel.h"
#include "UserMessageProxyModel.h"
#include "MegaApplication.h"

NotificationAlertController::NotificationAlertController(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(std::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
    , mGlobalListener(std::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
    , mUserMessagesModel(std::make_unique<UserMessageModel>(nullptr))
    , mUserMessagesProxyModel(std::make_unique<UserMessageProxyModel>(nullptr))
    , mAllUnseenAlerts(0)
{
    mUserMessagesProxyModel->setSourceModel(mUserMessagesModel.get());
    mUserMessagesProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.

    mMegaApi->addRequestListener(mDelegateListener.get());
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

void NotificationAlertController::onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e)
{
    Q_UNUSED(api)
    switch(request->getType())
    {
        case mega::MegaRequest::TYPE_GET_NOTIFICATIONS:
        {
            if (e->getErrorCode() == mega::MegaError::API_OK)
            {
                auto notificationList = request->getMegaNotifications();
                if (notificationList)
                {
                    mUserMessagesModel->processNotifications(notificationList);
                    mMegaApi->getLastReadNotification();
                }
            }
            break;
        }
        case mega::MegaRequest::TYPE_SET_ATTR_USER:
        case mega::MegaRequest::TYPE_GET_ATTR_USER:
        {
            if (e->getErrorCode() == mega::MegaError::API_OK
                    && request->getParamType() == mega::MegaApi::USER_ATTR_LAST_READ_NOTIFICATION
                    && mUserMessagesModel)
            {
                mUserMessagesModel->setLastSeenNotification(static_cast<uint32_t>(request->getNumber()));
                checkUseenNotifications();
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void NotificationAlertController::populateUserAlerts(mega::MegaUserAlertList* alertList)
{
    if (!alertList)
    {
        return;
    }

    mUserMessagesModel->processAlerts(alertList);
    checkUseenNotifications();

    // Used by DesktopNotifications because the current architecture
    emit userAlertsUpdated(alertList);
}

void NotificationAlertController::onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list)
{
    Q_UNUSED(api)

    if (MegaSyncApp->finished())
    {
        return;
    }

    if (list != nullptr)
    {
        // Process synchronously if list is provided
        populateUserAlerts(list);
    }
    else
    {
        // Process asynchronously if list is not provided
        ThreadPoolSingleton::getInstance()->push([this]()
        {
            // Retrieve the alerts in a separate thread
            mega::MegaUserAlertList* alertList = mMegaApi->getUserAlerts();

            // Queue the processing back to the main thread
            Utilities::queueFunctionInAppThread([this, alertList]()
            {
                populateUserAlerts(alertList);
                alertList->clear();
                delete alertList;
            });
        });
    }
}

void NotificationAlertController::reset()
{
    if (mUserMessagesModel)
    {
        mUserMessagesModel.reset();
    }

    if (mUserMessagesProxyModel)
    {
        mUserMessagesProxyModel.reset();
    }
}

bool NotificationAlertController::hasNotifications()
{
    return mUserMessagesModel->rowCount() > 0;
}

bool NotificationAlertController::hasElementsOfType(UserMessageType type)
{
    return mUserMessagesModel->hasAlertsOfType(type);
}

void NotificationAlertController::applyFilter(UserMessageType type)
{
    mUserMessagesProxyModel->setFilter(type);
}

void NotificationAlertController::requestNotifications() const
{
    if (MegaSyncApp->finished())
    {
        return;
    }

    mMegaApi->getNotifications();
}

void NotificationAlertController::checkUseenNotifications()
{
    if(!mUserMessagesModel)
    {
        return;
    }

    auto unseenAlerts = mUserMessagesModel->getUnseenNotifications();
    long long allUnseenAlerts = unseenAlerts[UserMessageType::ALL];
    if(mAllUnseenAlerts != allUnseenAlerts)
    {
        mAllUnseenAlerts = allUnseenAlerts;
        emit unseenAlertsChanged(unseenAlerts);
    }
}

void NotificationAlertController::ackSeenAlertsAndNotifications()
{
    // TODO: REMOVE AFTER TESTS
    //mMegaApi->setLastReadNotification(0);

    if (mAllUnseenAlerts > 0 && hasNotifications())
    {
        mMegaApi->acknowledgeUserAlerts();
        auto lastSeen = mUserMessagesModel->checkLocalLastSeenNotification();
        if(lastSeen > 0)
        {
            mMegaApi->setLastReadNotification(lastSeen);
        }
    }
}

QAbstractItemModel* NotificationAlertController::getModel() const
{
    return mUserMessagesProxyModel.get();
}
