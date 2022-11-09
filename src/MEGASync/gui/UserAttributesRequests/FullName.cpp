#include "FullName.h"

#include "megaapi.h"
#include "mega/types.h"
#include "AvatarWidget.h"
#include "MegaApplication.h"

namespace UserAttributes
{
// FULL NAME REQUEST
//
// The Full Name comprises the First Name AND the Last Name.
// The attribute is not considered resady while we don't have both.
// In case of error, and as a placeholder while we don't have both, the email is returned.
//
// The attributeReady() signal is sent when:
// - the attribute becomes ready (i.e. we just received either first or last name and we have both)
// - there was an error retrieving both the first name and the last name
//

void FullName::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    bool isFirstNameRequest(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_FIRSTNAME);
    bool isLastNameRequest(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_LASTNAME);

    if(isFirstNameRequest || isLastNameRequest)
    {
        if(e->getErrorCode() == mega::MegaError::API_OK)
        {
            if(isFirstNameRequest)
            {
                mFirstName = QString::fromUtf8(incoming_request->getText());
            }
            else if(isLastNameRequest)
            {
                mLastName = QString::fromUtf8(incoming_request->getText());
            }
        }

        if (isAttributeReady())
        {
            emit fullNameReady(getFullName());
            emit fullNameReadyRichText(getRichFullName());

            emit separateNamesReady(getFirstName(), getLastName());
            emit separateNamesReadyRichText(getFirstName().toHtmlEscaped(), getLastName().toHtmlEscaped());
        }
    }
}

void FullName::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_FIRSTNAME);
    requestUserAttribute(mega::MegaApi::USER_ATTR_LASTNAME);
}

AttributeRequest::RequestInfo FullName::fillRequestInfo()
{
    std::function<void()> firstNameRequest = [this]()
    {
        mFirstName.clear();
        MegaSyncApp->getMegaApi()->getUserAttribute(getEmail().toUtf8().constData(), mega::MegaApi::USER_ATTR_FIRSTNAME);
    };
    std::function<void()> lastNameRequest = [this]()
    {
        mLastName.clear();
        MegaSyncApp->getMegaApi()->getUserAttribute(getEmail().toUtf8().constData(), mega::MegaApi::USER_ATTR_LASTNAME);
    };

    QSharedPointer<ParamInfo> firstNameInfo(new ParamInfo(firstNameRequest));
    QSharedPointer<ParamInfo> lastNameInfo(new ParamInfo(lastNameRequest));

    ParamInfoMap paramInfo({{mega::MegaApi::USER_ATTR_FIRSTNAME, firstNameInfo},
                            {mega::MegaApi::USER_ATTR_LASTNAME, lastNameInfo}});

    RequestInfo ret(paramInfo, QMap<int, int>({{mega::MegaUser::CHANGE_TYPE_FIRSTNAME, mega::MegaApi::USER_ATTR_FIRSTNAME},
                                               {mega::MegaUser::CHANGE_TYPE_LASTNAME, mega::MegaApi::USER_ATTR_LASTNAME}}));
    return ret;
}

QString FullName::getFullName() const
{
    if(!isAttributeReady())
    {
        return getEmail();
    }

    return createFullName();
}

QString FullName::getRichFullName() const
{
    if(!isAttributeReady())
    {
        return getEmail();
    }

    return createFullName().toHtmlEscaped();
}

bool FullName::isAttributeReady() const
{
    return !isRequestPending() && (!getFirstName().isEmpty() || !getLastName().isEmpty());
}

const QString &FullName::getFirstName() const
{
    return mFirstName;
}

const QString &FullName::getLastName() const
{
    return mLastName;
}

QString FullName::createFullName() const
{
    QString fullName;

    if(!mFirstName.isEmpty())
    {
        fullName = mFirstName;

        if(!mLastName.isEmpty())
        {
            fullName.append(QString::fromUtf8(" %1").arg(mLastName));
        }
    }
    else
    {
        fullName = mLastName;
    }

    return fullName;
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
