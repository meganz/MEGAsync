#ifndef PROXYSTATSEVENTHANDLER_H
#define PROXYSTATSEVENTHANDLER_H

#include "IStatsEventHandler.h"

class ProxyStatsEventHandler : public IStatsEventHandler
{
public:
    using IStatsEventHandler::IStatsEventHandler;

    void sendEvent(AppStatsEvents::EventTypes type) override;
    void sendEvent(AppStatsEvents::EventTypes type,
                   const QStringList& args,
                   bool encode = false) override;

protected:
    void sendEvent(AppStatsEvents::EventTypes type, const char* message) override;

private:
    bool canSend() const;
    QByteArray encodeMessage(const QString& msg) const;

};

#endif // PROXYSTATSEVENTHANDLER_H
