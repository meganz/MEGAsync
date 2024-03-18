#include "EmailRequester.h"

#include "mega/types.h"
#include "MegaApplication.h"

EmailRequester::EmailRequester(mega::MegaHandle userHandle):
    mMegaApi(MegaSyncApp->getMegaApi()),
    mUserHandle(userHandle),
    mDelegateListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
{
    mMegaApi->addRequestListener(mDelegateListener.get());
}

void EmailRequester::onRequestFinish(mega::MegaApi*, mega::MegaRequest* request, mega::MegaError* error)
{
    if(request->getType() == mega::MegaRequest::TYPE_GET_USER_EMAIL)
    {
        QString userEmail;

        if(error->getErrorCode() == mega::MegaError::API_OK)
        {
            if (request->getEmail() != nullptr)
            {
                userEmail = QString::fromUtf8(request->getEmail());
            }
        }

        emit emailReceived(userEmail);

        deleteLater();
    }
}

void EmailRequester::requestEmail()
{
    const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();

    megaApi->getUserEmail(mUserHandle);
}

