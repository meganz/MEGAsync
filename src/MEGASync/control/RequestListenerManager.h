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

struct ListenerCallbacks
{
    QPointer<QObject> callbackClass;
    std::function<void(mega::MegaRequest *request)> onRequestStart;
    std::function<void(mega::MegaRequest *request, mega::MegaError* e)> onRequestFinish;
    std::function<void(mega::MegaRequest *request)> onRequestUpdate;
    std::function<void(mega::MegaRequest *request, mega::MegaError* e)> onRequestTemporaryError;
    bool removeAfterReqFinish = true;
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

private slots:
    void removeListener(ObserverRequestListener *listener);

private:
    RequestListenerManager();

    QMutex mListenerMutex;
    QList<std::shared_ptr<ObserverRequestListener>> mListeners;
};

#endif // REQUEST_LISTENER_MANAGER_H
