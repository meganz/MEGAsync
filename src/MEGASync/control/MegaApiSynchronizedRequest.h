#ifndef MEGAAPISYNCHRONIZEDREQUEST_H
#define MEGAAPISYNCHRONIZEDREQUEST_H

#include <megaapi.h>
#include <QTMegaRequestListener.h>

#include <qeventloop.h>

class MegaApiSynchronizedRequest
{
public:
    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<mega::MegaError> runRequestLambdaWithResult(REQUEST_FUNC func, mega::MegaApi* api, QPointer<const QObject> context, std::function<void(const mega::MegaRequest&, const mega::MegaError&)> resultFunc, Params&&... args)
    {
        std::shared_ptr<mega::MegaError> error(nullptr);
        QEventLoop eventLoop;
        func(std::forward<Params>(args)...,listenerMethod(api, context, resultFunc, error, eventLoop));
        eventLoop.exec();
        return error;
    }

    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<mega::MegaError> runRequestLambda(REQUEST_FUNC func, mega::MegaApi* api, Params&&... args)
    {
        return runRequestLambdaWithResult(func, api, nullptr, nullptr, std::forward<Params>(args)...);
    }

    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<mega::MegaError> runRequestWithResult(REQUEST_FUNC func, mega::MegaApi* api, QPointer<const QObject> context, std::function<void(const mega::MegaRequest&, const mega::MegaError&)> resultFunc,
        Params&&... args)
    {
        std::shared_ptr<mega::MegaError> error(nullptr);
        QEventLoop eventLoop;
        (api->*func)(std::forward<Params>(args)...,listenerMethod(api, context, resultFunc, error, eventLoop));
        eventLoop.exec();
        return error;
    }

    template<typename REQUEST_FUNC, typename... Params>
    static std::shared_ptr<mega::MegaError> runRequest(REQUEST_FUNC func, mega::MegaApi* api, Params&&... args)
    {
        return runRequestWithResult(func, api, nullptr, nullptr, std::forward<Params>(args) ...);
    }

private:
    static mega::OnFinishOneShot* listenerMethod(
        mega::MegaApi* api, QPointer<const QObject> context, std::function<void(const mega::MegaRequest&, const mega::MegaError&)> resultFunc, std::shared_ptr<mega::MegaError>& error, QEventLoop& eventLoop)
    {
        return new mega::OnFinishOneShot(api, context,
            [resultFunc, &error, &eventLoop](
                bool isContextValid, const mega::MegaRequest& request, const mega::MegaError& e)
            {
                eventLoop.quit();

                //In case of error, move to OS trash
                if(e.getErrorCode() != mega::MegaError::API_OK)
                {
                    error.reset(e.copy());
                }

                if(isContextValid && resultFunc)
                {
                    resultFunc(request, e);
                }
            });
    }
};

#endif // MEGAAPISYNCHRONIZEDREQUEST_H
