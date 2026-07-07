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
                                                    "The uploaded file could not be found"),
                        false);
        return;
    }

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [this](MegaRequest* request, MegaError* error)
        {
            if (error->getErrorCode() != MegaError::API_OK)
            {
                const bool authExpired = error->getErrorCode() == MegaError::API_ESID ||
                                         error->getErrorCode() == MegaError::API_EBLOCKED;
                emit linkFailed(QString::fromUtf8(error->getErrorString()), authExpired);
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
