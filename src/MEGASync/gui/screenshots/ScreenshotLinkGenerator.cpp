#include "ScreenshotLinkGenerator.h"

#include "RequestListenerManager.h"

#include <QCoreApplication>

#include <memory>

using namespace mega;

ScreenshotLinkGenerator::ScreenshotLinkGenerator(MegaApi* megaApi, QObject* parent):
    QObject(parent),
    mMegaApi(megaApi)
{}

void ScreenshotLinkGenerator::generateLink(MegaHandle nodeHandle)
{
    std::unique_ptr<MegaNode> node(mMegaApi->getNodeByHandle(nodeHandle));
    if (!node)
    {
        emit linkFailed(QCoreApplication::translate("ScreenshotLinkGenerator",
                                                    "The uploaded file could not be found"));
        return;
    }

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [this](MegaRequest* request, MegaError* error)
        {
            if (error->getErrorCode() != MegaError::API_OK)
            {
                // The app's global TYPE_EXPORT handler already reports this
                // to the user ("Error getting link: %1"); just log it.
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                             QString::fromUtf8("Screenshot link export failed: %1")
                                 .arg(QString::fromUtf8(error->getErrorString()))
                                 .toUtf8()
                                 .constData());
                return;
            }

            emit linkReady(QString::fromUtf8(request->getLink()));
        });

    mMegaApi->exportNode(node.get(),
                         0 /*no expiry*/,
                         false /*writable*/,
                         false /*megaHosted*/,
                         listener.get());
}
