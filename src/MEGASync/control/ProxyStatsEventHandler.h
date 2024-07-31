#ifndef PROXYSTATSEVENTHANDLER_H
#define PROXYSTATSEVENTHANDLER_H

#include "StatsEventHandler.h"

class ProxyStatsEventHandler : public StatsEventHandler
{
    Q_OBJECT

public:
    using StatsEventHandler::StatsEventHandler;

    Q_INVOKABLE void sendEvent(AppStatsEvents::EventType type,
                               const QStringList& args = QStringList(),
                               bool encode = false) override;

    Q_INVOKABLE void sendTrackedEvent(AppStatsEvents::EventType type,
                                      bool fromInfoDialog = false) override;

    Q_INVOKABLE void sendTrackedEventArg(AppStatsEvents::EventType type,
                                         const QStringList& args = QStringList(),
                                         bool fromInfoDialog = false) override;

    void sendTrackedEvent(AppStatsEvents::EventType type,
                          const QObject* senderObj,
                          const QObject* expectedObj,
                          bool fromInfoDialog = false) override;

protected:
    void send(AppStatsEvents::EventType type,
              const QString& message,
              bool addJourneyId = false,
              const char* viewId = nullptr) override;

private:
    bool canSend() const;
    QString encodeMessage(const QString& msg) const;
    void updateTrackInfo(bool fromInfoDialog = false);

};

#endif // PROXYSTATSEVENTHANDLER_H
