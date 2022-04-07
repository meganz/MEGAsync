#include "UserAttributesRequests.h"

#include "megaapi.h"
#include "mega/types.h"

namespace UserAttributes
{
//FUL NAME REQUEST
//
//
void FullNameAttributeRequest::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError*)
{
    if(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_FIRSTNAME)
    {
        mFirstName = QString::fromUtf8(incoming_request->getText());

        if(mFirstName.isEmpty())
        {
            mFirstName = QString::fromUtf8(incoming_request->getEmail());
        }

        if(!mLastName.isEmpty())
        {
            emit attributeReady();
        }
    }
    else if(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_LASTNAME)
    {
        mLastName = QString::fromUtf8(incoming_request->getText());

        if(!mFirstName.isEmpty())
        {
            emit attributeReady();
        }
    }
}

void FullNameAttributeRequest::requestAttribute()
{
    const auto megaApp = static_cast<MegaApplication*>(qApp);
    megaApp->getMegaApi()->getUserAttribute(mUserEmail.toStdString().c_str(),mega::ATTR_FIRSTNAME);
    megaApp->getMegaApi()->getUserAttribute(mUserEmail.toStdString().c_str(),mega::ATTR_LASTNAME);
}

void FullNameAttributeRequest::updateAttributes(mega::MegaUser *user)
{
    if (user->hasChanged(mega::MegaUser::CHANGE_TYPE_FIRSTNAME))
    {
        const auto megaApp = static_cast<MegaApplication*>(qApp);
        megaApp->getMegaApi()->getUserAttribute(mUserEmail.toStdString().c_str(),mega::ATTR_FIRSTNAME);
    }
    else if(user->hasChanged(mega::MegaUser::CHANGE_TYPE_LASTNAME))
    {
        const auto megaApp = static_cast<MegaApplication*>(qApp);
        megaApp->getMegaApi()->getUserAttribute(mUserEmail.toStdString().c_str(),mega::ATTR_LASTNAME);
    }
}

QString FullNameAttributeRequest::getFullName()
{
    if(mFirstName.isEmpty() && mLastName.isEmpty())
    {
        return QString();
    }

    return QString(QString::fromUtf8("%1 %2")).arg(mFirstName).arg(mLastName);
}

std::shared_ptr<FullNameAttributeRequest> FullNameAttributeRequest::requestFullName(const char *user_email)
{
    return UserAttributesManager::instance().requestAttribute<FullNameAttributeRequest>(user_email);
}


//AVATAR REQUEST
//
//
std::shared_ptr<AvatarAttributeRequest> AvatarAttributeRequest::requestAvatar(const char *user_email)
{
    return UserAttributesManager::instance().requestAttribute<AvatarAttributeRequest>(user_email);
}

void AvatarAttributeRequest::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError*)
{
    if(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_AVATAR)
    {
        mFilePath = QString::fromUtf8(incoming_request->getFile());
        #ifdef WIN32
        if (mFilePath.startsWith(QString::fromUtf8("\\\\?\\")))
        {
            mFilePath = mFilePath.mid(4);
        }
        #endif
        emit attributeReady();
    }
}

void AvatarAttributeRequest::requestAttribute()
{
    const auto megaApp = static_cast<MegaApplication*>(qApp);
    megaApp->getMegaApi()->getUserAvatar(mUserEmail.toStdString().c_str(),
                                         Utilities::getAvatarPath(QString::fromUtf8(mUserEmail.toStdString().c_str())).toUtf8().constData());
}

void AvatarAttributeRequest::updateAttributes(mega::MegaUser *user)
{
    if (user->hasChanged(mega::MegaUser::CHANGE_TYPE_AVATAR))
    {
        const auto megaApp = static_cast<MegaApplication*>(qApp);
        megaApp->getMegaApi()->getUserAttribute(mUserEmail.toStdString().c_str(),mega::ATTR_AVATAR);
    }
}

QPixmap AvatarAttributeRequest::GetPixmap(int diameter)
{
    return QPixmap(); //return AvatarPixmap::maskFromImagePath(diameter);
}
}
