#ifndef SCREENSHOT_ACCESS_CONTROLLER_H
#define SCREENSHOT_ACCESS_CONTROLLER_H

#include "megaapi.h"

#include <QObject>

class ScreenshotAccessController: public QObject
{
    Q_OBJECT

public:
    explicit ScreenshotAccessController(QObject* parent = 0);

signals:

private:
    mega::MegaApi* mMegaApi;
};

#endif
