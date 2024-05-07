#ifndef PROXYSTATSEVENTHANDLER_H
#define PROXYSTATSEVENTHANDLER_H

#include "IStatsEventHandler.h"

class ProxyStatsEventHandler : public IStatsEventHandler
{
    Q_OBJECT

public:
    using IStatsEventHandler::IStatsEventHandler;

    Q_INVOKABLE void sendEvent(AppStatsEvents::EventType type,
                               const QStringList& args = QStringList(),
                               bool encode = false) override;

    Q_INVOKABLE void sendTrackedEvent(AppStatsEvents::EventType type,
                                      bool fromInfoDialog = false) override;

    void sendTrackedEvent(AppStatsEvents::EventType type,
                          const QObject* senderObj,
                          const QObject* expectedObj,
                          bool fromInfoDialog = false) override;

protected:
    void sendEvent(AppStatsEvents::EventType type,
                   const char* message,
                   bool addJourneyId = false,
                   const char* viewId = nullptr) override;

private:
    bool canSend() const;
    QByteArray encodeMessage(const QString& msg) const;

};

#endif // PROXYSTATSEVENTHANDLER_H
