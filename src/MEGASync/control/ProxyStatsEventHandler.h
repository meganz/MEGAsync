#ifndef PROXYSTATSEVENTHANDLER_H
#define PROXYSTATSEVENTHANDLER_H

#include "IStatsEventHandler.h"

class ProxyStatsEventHandler : public IStatsEventHandler
{
    Q_OBJECT

public:
    using IStatsEventHandler::IStatsEventHandler;

    Q_INVOKABLE void sendEvent(AppStatsEvents::EventTypes type,
                               const QStringList& args = QStringList(),
                               bool encode = false) override;
    Q_INVOKABLE void sendTrackedEvent(int type) override;

protected:
    void sendEvent(AppStatsEvents::EventTypes type,
                   const char* message,
                   bool addJourneyId = false,
                   const char* viewId = nullptr) override;

private:
    bool canSend() const;
    QByteArray encodeMessage(const QString& msg) const;

};

#endif // PROXYSTATSEVENTHANDLER_H
