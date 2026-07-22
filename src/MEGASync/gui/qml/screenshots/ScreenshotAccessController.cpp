#include "ScreenshotAccessController.h"

#include "MegaApplication.h"

using namespace mega;

ScreenshotAccessController::ScreenshotAccessController(QObject* parent):
    QObject(parent),
    mMegaApi(MegaSyncApp->getMegaApi())
{}
