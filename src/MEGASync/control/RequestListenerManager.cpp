#include "RequestListenerManager.h"
#include <QDir>
#include "MegaApplication.h"

using namespace mega;

ObserverRequestListener::ObserverRequestListener(const ListenerCallbacks& callbacks)
    : mCallbacks(callbacks)
    , mDelegateListener(std::make_shared<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
{
    // Connect to destroyed signal of callbackClass
    if (mCallbacks.callbackClass)
    {
        connect(mCallbacks.callbackClass, &QObject::destroyed, this, &ObserverRequestListener::objectDestroyed);
    }
}

void ObserverRequestListener::objectDestroyed(QObject*)
{
    emit removeListener(this);
}

std::shared_ptr<mega::QTMegaRequestListener> ObserverRequestListener::getDelegateListener() const
{
    return mDelegateListener;
}

void ObserverRequestListener::onRequestStart(MegaApi*, MegaRequest* request)
{
    if (mCallbacks.onRequestStart && mCallbacks.callbackClass)
    {
        mCallbacks.onRequestStart(request);
    }
}

void ObserverRequestListener::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* error)
{
    if (mCallbacks.onRequestFinish && mCallbacks.callbackClass)
    {
        mCallbacks.onRequestFinish(request, error);
    }

    if (!mCallbacks.callbackClass || mCallbacks.removeAfterReqFinish)
    {
        // Observer has been destroyed or it has finished and it need to be removed
        emit removeListener(this);
    }
}

void ObserverRequestListener::onRequestUpdate(MegaApi*, MegaRequest* request)
{
    if (mCallbacks.onRequestUpdate && mCallbacks.callbackClass)
    {
        mCallbacks.onRequestUpdate(request);
    }
}

void ObserverRequestListener::onRequestTemporaryError(MegaApi*,
                                                      MegaRequest* request,
                                                      MegaError* error)
{
    if (mCallbacks.onRequestTemporaryError && mCallbacks.callbackClass)
    {
        mCallbacks.onRequestTemporaryError(request, error);
    }
}

// ----------------------------------------------------------------------------

RequestListenerManager::RequestListenerManager()
{
}

std::shared_ptr<QTMegaRequestListener> RequestListenerManager::registerAndGetListener(const ListenerCallbacks& callbacks)
{
    QMutexLocker locker(&mListenerMutex);
    auto listener = std::make_shared<ObserverRequestListener>(callbacks);
    mListeners.push_back(listener);
    connect(listener.get(), &ObserverRequestListener::removeListener, this, &RequestListenerManager::removeListener);
    return listener->getDelegateListener();
}

// std::shared_ptr<mega::QTMegaRequestListener> RequestListenerManager::registerAndGetFinishListener(const onRequestFinishOnlyListenerCallback& callback)
// {
//     ListenerCallbacks callbacks;
//     callbacks.callbackClass = callback.callbackClass;
//     callbacks.onRequestFinish = callback.onRequestFinish;
//     callbacks.removeAfterReqFinish = callback.removeAfterReqFinish;

//     return registerAndGetListener(callbacks);
// }
// template<typename T>
// std::shared_ptr<mega::QTMegaRequestListener> RequestListenerManager::registerAndGetFinishListener(T* callbackClass)
// {
//     ListenerCallbacks callbacks;
//     callbacks.callbackClass = callbackClass;
//     callbacks.onRequestFinish = [callbackClass](mega::MegaRequest* request, mega::MegaError* error) {
//         callbackClass->onRequestFinish(request, error);
//     };
//     callbacks.removeAfterReqFinish = true;  // Assuming it should be removed after finish

//     return registerAndGetListener(callbacks);
// }

void RequestListenerManager::removeListener(ObserverRequestListener *listener)
{
    QMutexLocker locker(&mListenerMutex);
    auto it = std::find_if(mListeners.begin(), mListeners.end(),
                           [listener](const std::shared_ptr<ObserverRequestListener>& l) {
                               return l.get() == listener;
                           });

    if (it != mListeners.end())
    {
        disconnect((*it).get(), &ObserverRequestListener::removeListener, this, &RequestListenerManager::removeListener);
        mListeners.erase(it);
    }
}


