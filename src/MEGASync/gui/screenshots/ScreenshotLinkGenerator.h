#ifndef SCREENSHOT_LINK_GENERATOR_H
#define SCREENSHOT_LINK_GENERATOR_H

#include "megaapi.h"

#include <QObject>
#include <QString>

// Exports an uploaded node as a MEGA public link. The clipboard copy, the
// "link copied" notification and SDK error reporting are owned by the app's
// standard TYPE_EXPORT handling (MegaApplication::onRequestFinish), like any
// other export; linkFailed only covers the local case where no export
// request was made.
class ScreenshotLinkGenerator: public QObject
{
    Q_OBJECT

public:
    explicit ScreenshotLinkGenerator(mega::MegaApi* megaApi, QObject* parent = nullptr);

    void generateLink(mega::MegaHandle nodeHandle);

signals:
    void linkReady(const QString& link);
    void linkFailed(const QString& message);

private:
    mega::MegaApi* mMegaApi;
};

#endif // SCREENSHOT_LINK_GENERATOR_H
