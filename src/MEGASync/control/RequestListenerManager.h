#ifndef REQUEST_LISTENER_MANAGER_H
#define REQUEST_LISTENER_MANAGER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QPointer>
#include <functional>
#include <memory>
#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include <mutex>

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

private slots:
    void objectDestroyed(QObject *obj);

private:
    ListenerCallbacks mCallbacks;
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
};

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
    QList<std::shared_ptr<ObserverRequestListener>> mListeners;
};

#endif // REQUEST_LISTENER_MANAGER_H
