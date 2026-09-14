#include "UserMessageController.h"

#include "MegaApplication.h"
#include "RequestListenerManager.h"

#include <QPointer>
#include <QTimer>

const int USER_ALERT_PROCESS_DELAY_IN_SECONDS = 1;

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
            if (error->getErrorCode() == mega::MegaError::API_OK
                    || error->getErrorCode() == mega::MegaError::API_ENOENT)
            {
                // If empty list (API_ENOENT), clear notifications.
                auto notificationList = request->getMegaNotifications();
                mUserMessagesModel->processNotifications(notificationList);
                mMegaApi->getLastReadNotification(mDelegateListener.get());
                emit userMessagesReceived();
            }
            break;
        }
        case mega::MegaRequest::TYPE_SET_ATTR_USER:
        case mega::MegaRequest::TYPE_GET_ATTR_USER:
        {
            if (error->getErrorCode() == mega::MegaError::API_ENOENT)
            {
                checkUseenNotifications();
            }
            else if (error->getErrorCode() == mega::MegaError::API_OK &&
                     request->getParamType() == mega::MegaApi::USER_ATTR_LAST_READ_NOTIFICATION &&
                     mUserMessagesModel)
            {
                mUserMessagesModel->setLastSeenNotification(
                    static_cast<uint32_t>(request->getNumber()));
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
        // Wait for the shared folders to be undecrypted before processing. This callback already
        // runs on the app thread (QTMegaGlobalListener posts it), so a context-bound timer replaces
        // the former ThreadPool sleep + queueFunctionInAppThread(qApp) chain: Qt drops the pending
        // call if the controller is destroyed meanwhile (logout / session change), instead of
        // running it on a freed object.
        std::shared_ptr<mega::MegaUserAlertList> auxList(list->copy());
        QTimer::singleShot(USER_ALERT_PROCESS_DELAY_IN_SECONDS * 1000,
                           this,
                           [this, auxList]()
                           {
                               // Process synchronously if list is provided
                               populateUserAlerts(auxList.get());
                               auxList->clear();
                           });
    }
    else
    {
        // Process asynchronously if list is not provided: getUserAlerts() blocks, so it stays on
        // the pool. The controller may be destroyed (logout / session change) before the task
        // runs, so the pool lambda must not read members of `this` (api pointer copied now) and
        // the app-thread lambda only touches the controller behind a QPointer guard. The guard is
        // checked on the app thread only: that is the thread that destroys the object, so checking
        // it from the pool would race the very destruction it protects against.
        QPointer<UserMessageController> guard(this);
        mega::MegaApi* megaApi = mMegaApi;
        ThreadPoolSingleton::getInstance()->push(
            [guard, megaApi]()
            {
                // Retrieve the alerts in a separate thread
                std::shared_ptr<mega::MegaUserAlertList> alertList(megaApi->getUserAlerts());

                // Queue the processing back to the main thread
                Utilities::queueFunctionInAppThread(
                    [guard, alertList]()
                    {
                        if (!guard)
                        {
                            return;
                        }

                        guard->populateUserAlerts(alertList.get());
                        alertList->clear();
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
