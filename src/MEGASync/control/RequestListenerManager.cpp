#include "RequestListenerManager.h"

#include "MegaApplication.h"

#include <QDir>
#include <QPointer>

using namespace mega;

ObserverRequestListener::ObserverRequestListener(const ListenerCallbacks& callbacks):
    mCallbacks(callbacks),
    mDelegateListener(
        std::make_shared<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
{}

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
    if (mCallbacks.onRequestFinish &&
        (mCallbacks.isSynchronousOneShotReq || mCallbacks.callbackClass))
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

RequestListenerManager::RequestListenerManager() {}

RequestListenerManager::~RequestListenerManager()
{
    qDeleteAll(mListeners);
}

std::shared_ptr<QTMegaRequestListener> RequestListenerManager::registerAndGetListener(const ListenerCallbacks& callbacks)
{
    QMutexLocker locker(&mListenerMutex);

    QPointer<ObserverRequestListener> listener(new ObserverRequestListener(callbacks));
    mListeners.push_back(listener);
    connect(listener,
            &ObserverRequestListener::removeListener,
            this,
            &RequestListenerManager::removeListener);

    return listener->getDelegateListener();
}

void RequestListenerManager::removeListener(ObserverRequestListener *listener)
{
    QMutexLocker locker(&mListenerMutex);

    mListeners.removeOne(listener);
    listener->deleteLater();
}


