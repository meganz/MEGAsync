#include "EmailRequester.h"

#include "MegaApplication.h"

EmailRequester::EmailRequester(mega::MegaUserAlert* alert):
    mAlert(alert)
{}

EmailRequester::~EmailRequester()
{
    delete mAlert;
}

void EmailRequester::onRequestFinish(mega::MegaApi*, mega::MegaRequest* request, mega::MegaError* error)
{
    if(request->getType() == mega::MegaRequest::TYPE_GET_USER_EMAIL)
    {
        if(error->getErrorCode() == mega::MegaError::API_OK)
        {
            QString userEmail;

            if (request->getEmail() != nullptr)
            {
                userEmail = QString::fromUtf8(request->getEmail());
            }

            emit emailReceived(mAlert, userEmail);
        }

        delete this;
    }
}

void EmailRequester::requestEmail()
{
    const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();

    megaApi->getUserEmail(mAlert->getUserHandle(), this);
}

