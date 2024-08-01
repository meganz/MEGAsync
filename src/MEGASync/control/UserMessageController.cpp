#include "UserMessageController.h"

#include "RequestListenerManager.h"
#include "MegaApplication.h"

UserMessageController::UserMessageController(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mGlobalListener(std::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
    , mUserMessagesModel(std::make_unique<UserMessageModel>(nullptr))
    , mUserMessagesProxyModel(std::make_unique<UserMessageProxyModel>(nullptr))
    , mAllUnseenAlerts(0)
{
    mUserMessagesProxyModel->setSourceModel(mUserMessagesModel.get());
    mUserMessagesProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.

    mDelegateListener = RequestListenerManager::instance().registerAndGetFinishListener(this);
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

void UserMessageController::onRequestFinish(mega::MegaRequest* request, mega::MegaError* error)
{
    switch(request->getType())
    {
        case mega::MegaRequest::TYPE_GET_NOTIFICATIONS:
        {
            if (error->getErrorCode() == mega::MegaError::API_OK)
            {
                auto notificationList = request->getMegaNotifications();
                if (notificationList)
                {
                    mUserMessagesModel->processNotifications(notificationList);
                    mMegaApi->getLastReadNotification();
                    emit userMessagesReceived();
                }
            }
            break;
        }
        case mega::MegaRequest::TYPE_SET_ATTR_USER:
        case mega::MegaRequest::TYPE_GET_ATTR_USER:
        {
            if (error->getErrorCode() == mega::MegaError::API_OK
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

void UserMessageController::populateUserAlerts(mega::MegaUserAlertList* alertList)
{
    if (!alertList)
    {
        return;
    }

    mUserMessagesModel->processAlerts(alertList);
    checkUseenNotifications();

    emit userMessagesReceived();

    // Used by DesktopNotifications because the current architecture
    emit userAlertsUpdated(alertList);
}

void UserMessageController::onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list)
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

bool UserMessageController::hasNotifications()
{
    return mUserMessagesModel->rowCount() > 0;
}

bool UserMessageController::hasElementsOfType(MessageType type)
{
    return mUserMessagesModel->hasAlertsOfType(type);
}

void UserMessageController::applyFilter(MessageType type)
{
    mUserMessagesProxyModel->setFilter(type);
}

void UserMessageController::requestNotifications() const
{
    if (MegaSyncApp->finished())
    {
        return;
    }

    mMegaApi->getNotifications(mDelegateListener.get());
}

void UserMessageController::checkUseenNotifications()
{
    if(!mUserMessagesModel)
    {
        return;
    }

    auto unseenAlerts = mUserMessagesModel->getUnseenNotifications();
    long long allUnseenAlerts = unseenAlerts[MessageType::ALL];
    if(mAllUnseenAlerts != allUnseenAlerts)
    {
        mAllUnseenAlerts = allUnseenAlerts;
        emit unseenAlertsChanged(unseenAlerts);
    }
}

void UserMessageController::ackSeenUserMessages()
{
    if (mAllUnseenAlerts > 0 && hasNotifications())
    {
        mMegaApi->acknowledgeUserAlerts(mDelegateListener.get());
        auto lastSeen = mUserMessagesModel->checkLocalLastSeenNotification();
        if(lastSeen > 0)
        {
            mMegaApi->setLastReadNotification(lastSeen, mDelegateListener.get());
        }
    }
}

QAbstractItemModel* UserMessageController::getModel() const
{
    return mUserMessagesProxyModel.get();
}

bool UserMessageController::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        requestNotifications();
    }

    return QObject::event(event);
}
