#ifndef SCREENSHOT_LINK_GENERATOR_H
#define SCREENSHOT_LINK_GENERATOR_H

#include "megaapi.h"

#include <QObject>
#include <QString>

// Exports an uploaded node as a MEGA public link. Produces the link only;
// clipboard/notification side effects are the caller's responsibility so this
// stays a thin, reusable wrapper over MegaApi::exportNode.
class ScreenshotLinkGenerator: public QObject
{
    Q_OBJECT

public:
    explicit ScreenshotLinkGenerator(mega::MegaApi* megaApi, QObject* parent = nullptr);

    void generateLink(mega::MegaHandle nodeHandle);

signals:
    void linkReady(const QString& link);
    void linkFailed(const QString& message, bool authExpired);

private:
    mega::MegaApi* mMegaApi;
};

#endif // SCREENSHOT_LINK_GENERATOR_H
