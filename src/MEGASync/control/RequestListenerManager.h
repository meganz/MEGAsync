#ifndef REQUEST_LISTENER_MANAGER_H
#define REQUEST_LISTENER_MANAGER_H

#include "megaapi.h"
#include "QTMegaListener.h"
#include "QTMegaRequestListener.h"

#include <functional>
#include <memory>
#include <mutex>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QPointer>

struct onRequestFinishOnlyListenerCallback
{
    QPointer<QObject> callbackClass;
    bool removeAfterReqFinish = false;
};

struct ListenerCallbacks
{
    QPointer<QObject> callbackClass = nullptr;
    std::function<void(mega::MegaRequest *request)> onRequestStart = nullptr;
    std::function<void(mega::MegaRequest *request, mega::MegaError* e)> onRequestFinish = nullptr;
    std::function<void(mega::MegaRequest *request)> onRequestUpdate = nullptr;
    std::function<void(mega::MegaRequest *request, mega::MegaError* e)> onRequestTemporaryError = nullptr;
    bool removeAfterReqFinish = false;
    bool isSynchronousOneShotReq = false;
};

class ObserverRequestListener : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    ObserverRequestListener(const ListenerCallbacks& callbacks);

    std::shared_ptr<mega::QTMegaRequestListener> getDelegateListener() const;

    void onRequestStart(mega::MegaApi* api, mega::MegaRequest* request) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* error) override;
    void onRequestUpdate(mega::MegaApi* api, mega::MegaRequest* request) override;
    void onRequestTemporaryError(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* error) override;

signals:
    void removeListener(ObserverRequestListener *listener);

private:
    ListenerCallbacks mCallbacks;
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
};

/* Class usage
 *
 * This class is used to create request listeners, and it manages their life.
 * There are two ways to use request listeners:
 * Adding them with megaApi::addRequestListener or using them in one of the different requests.
 * In the first case, it would not be strictly necessary to use this manager,
 * since the request listener destructor itself calls the megaApi::removeRequestListener function,
 * avoiding crashes, so this manager would only be used to facilitate its creation.
 * In the second case, the use of the manager is recommended to ensure that the listener
 * is not destroyed while the request is in progress.
 * For this, it is recommended to create the listener just before using it,
 * to minimize the case in which the class that uses it is destroyed before the SDK processes the
 * request, and not to save it as a member of the class.
 * In case that happens, it has been preferred not to delete the listener and accumulate
 * instances, than to suffer a crash, as it is an edge case.
 *
 */

class RequestListenerManager : public QObject
{
    Q_OBJECT

public:
    static RequestListenerManager& instance()
    {
        static std::unique_ptr<RequestListenerManager> instance;
        static std::once_flag flag;

        std::call_once(flag, [&]() {
            instance.reset(new RequestListenerManager());
        });

        return *instance;
    }

    RequestListenerManager(const RequestListenerManager&) = delete;
    RequestListenerManager& operator=(const RequestListenerManager&) = delete;

    ~RequestListenerManager();

    std::shared_ptr<mega::QTMegaRequestListener> registerAndGetListener(const ListenerCallbacks& callbacks);

    template<typename T>
    std::shared_ptr<mega::QTMegaRequestListener>
    registerAndGetFinishListener(T* callbackClass, bool removeAfterReqFinish = false)
    {
        ListenerCallbacks callbacks;
        callbacks.callbackClass = callbackClass;
        callbacks.onRequestFinish = [callbackClass](::mega::MegaRequest* request, ::mega::MegaError* error) {
            callbackClass->onRequestFinish(request, error);
        };
        callbacks.removeAfterReqFinish = removeAfterReqFinish;

        return registerAndGetListener(callbacks);
    }

    //! This function will primarily be used for "single shot" purposes:
    //! One request, one "onRequestFinish" callback
    //! Hence, the default value of removeAfterReqFinish is "true"
    template<typename T>
    std::shared_ptr<::mega::QTMegaRequestListener> registerAndGetCustomFinishListener(
        T* callbackClass,
        std::function<void(::mega::MegaRequest*, ::mega::MegaError*)> onRequestFinish,
        bool removeAfterReqFinish = true)
    {
        ListenerCallbacks callbacks;
        callbacks.callbackClass = callbackClass;
        callbacks.onRequestFinish = onRequestFinish;
        callbacks.removeAfterReqFinish = removeAfterReqFinish;
        return registerAndGetListener(callbacks);
    }

    std::shared_ptr<::mega::QTMegaRequestListener> registerAndGetSynchronousFinishListener(
        std::function<void(::mega::MegaRequest*, ::mega::MegaError*)> onRequestFinish)
    {
        ListenerCallbacks callbacks;
        callbacks.callbackClass = nullptr;
        callbacks.onRequestFinish = onRequestFinish;
        callbacks.removeAfterReqFinish = true;
        callbacks.isSynchronousOneShotReq = true;
        return registerAndGetListener(callbacks);
    }

private slots:
    void removeListener(ObserverRequestListener *listener);

private:
    RequestListenerManager();

    QMutex mListenerMutex;

    QList<QPointer<ObserverRequestListener>> mListeners;
};

#endif // REQUEST_LISTENER_MANAGER_H
