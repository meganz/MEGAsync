#include "FullName.h"

#include "megaapi.h"
#include "mega/types.h"
#include "AvatarWidget.h"
#include "MegaApplication.h"

namespace UserAttributes
{
//FUL NAME REQUEST
//
//
void FullName::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    bool isFirstNameRequest(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_FIRSTNAME);
    bool isLastNameRequest(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_LASTNAME);

    if(isFirstNameRequest
            || isLastNameRequest)
    {
        if(e->getErrorCode() == mega::MegaError::API_OK)
        {
            if(isFirstNameRequest)
            {
                mFirstName = QString::fromUtf8(incoming_request->getText());

                if(mFirstName.isEmpty())
                {
                    mFirstName = QString::fromUtf8(incoming_request->getEmail());
                }
            }
            else if(isLastNameRequest)
            {
                mLastName = QString::fromUtf8(incoming_request->getText());
            }

            if(isAttributeReady())
            {
                emit attributeReady(getFullName());
                emit attributeReadyRichText(getRichFullName());
            }
        }
        else
        {
            //If you get an error getting the full name, at least show the email
            emit attributeReady(getFullName());
            emit attributeReadyRichText(getRichFullName());
        }
    }
}

void FullName::requestAttribute()
{
    requestFirstNameAttribute();
    requestLastNameAttribute();
}

void FullName::updateAttributes(mega::MegaUser *user)
{
    bool hasFirstnameChanged = user->hasChanged(mega::MegaUser::CHANGE_TYPE_FIRSTNAME);
    bool hasLastnameChanged = user->hasChanged(mega::MegaUser::CHANGE_TYPE_LASTNAME);

    if (hasFirstnameChanged)
    {
        requestFirstNameAttribute();
    }

    if(hasLastnameChanged)
    {
        requestLastNameAttribute();
    }
}

void FullName::requestFirstNameAttribute()
{
    mFirstName.clear();
    MegaSyncApp->getMegaApi()->getUserAttribute(mUserEmail.toUtf8().constData(),mega::ATTR_FIRSTNAME);
}

void FullName::requestLastNameAttribute()
{
    mLastName.clear();
    MegaSyncApp->getMegaApi()->getUserAttribute(mUserEmail.toUtf8().constData(),mega::ATTR_LASTNAME);
}


QString FullName::getFullName() const
{
    if(!isAttributeReady())
    {
        return getEmail();
    }

    return QString(QString::fromUtf8("%1 %2")).arg(mFirstName).arg(mLastName);
}

QString FullName::getRichFullName() const
{
    auto text = QString::fromUtf8("%1 %2").arg(mFirstName).arg(mLastName);
    return text.toHtmlEscaped();
}

bool FullName::isAttributeReady() const
{
    return !mFirstName.isEmpty() && !mLastName.isEmpty();
}

const QString &FullName::getFirstName() const
{
    return mFirstName;
}

const QString &FullName::getLastName() const
{
    return mLastName;
}

std::shared_ptr<const FullName> FullName::requestFullName(const char *user_email)
{
    if(user_email)
    {
        return UserAttributesManager::instance().requestAttribute<FullName>(user_email);
    }
    else
    {
        return nullptr;
    }
}

}//end namespace UserAttributes
