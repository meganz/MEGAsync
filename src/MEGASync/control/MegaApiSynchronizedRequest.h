#ifndef MEGAAPISYNCHRONIZEDREQUEST_H
#define MEGAAPISYNCHRONIZEDREQUEST_H

#include <megaapi.h>
#include <QTMegaRequestListener.h>
#include "RequestListenerManager.h"
#include <qeventloop.h>

class MegaApiSynchronizedRequest
{
public:
    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<::mega::MegaError> runRequestLambdaWithResult(
        REQUEST_FUNC func,
        ::mega::MegaApi* api,
        std::function<void(::mega::MegaRequest*, ::mega::MegaError*)> resultFunc,
        Params&&... args)
    {
        std::shared_ptr<::mega::MegaError> error(nullptr);
        QEventLoop eventLoop;
        func(std::forward<Params>(args)...,listenerMethod(api, resultFunc, error, eventLoop));
        eventLoop.exec();
        return error;
    }

    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<::mega::MegaError> runRequestLambda(REQUEST_FUNC func,
                                                               ::mega::MegaApi* api,
                                                               Params&&... args)
    {
        return runRequestLambdaWithResult(func, api, nullptr, std::forward<Params>(args)...);
    }

    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<::mega::MegaError> runRequestWithResult(
        REQUEST_FUNC func,
        ::mega::MegaApi* api,
        std::function<void(::mega::MegaRequest*, ::mega::MegaError*)> resultFunc,
        Params&&... args)
    {
        std::shared_ptr<::mega::MegaError> error(nullptr);
        QEventLoop eventLoop;
        (api->*func)(std::forward<Params>(args)...,listenerMethod(api, resultFunc, error, eventLoop));
        eventLoop.exec();
        return error;
    }

    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<::mega::MegaError> runRequest(REQUEST_FUNC func,
                                                         ::mega::MegaApi* api,
                                                         Params&&... args)
    {
        return runRequestWithResult(func, api, nullptr, std::forward<Params>(args) ...);
    }

private:
    static mega::MegaRequestListener*
        listenerMethod(mega::MegaApi* api,
                       std::function<void(mega::MegaRequest*, mega::MegaError*)> resultFunc,
                       std::shared_ptr<mega::MegaError>& error,
                       QEventLoop& eventLoop)
    {
        return RequestListenerManager::instance().registerAndGetSynchronousFinishListener(
            [resultFunc, &error, &eventLoop](mega::MegaRequest* request, mega::MegaError* e) {
                eventLoop.quit();

                //In case of error, move to OS trash
                if (e->getErrorCode() != mega::MegaError::API_OK)
                {
                    error.reset(e->copy());
                }

                if (resultFunc)
                {
                    resultFunc(request, e);
                }
            }).get();
    }
};

#endif // MEGAAPISYNCHRONIZEDREQUEST_H
