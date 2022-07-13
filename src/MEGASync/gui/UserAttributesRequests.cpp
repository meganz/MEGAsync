#include "UserAttributesRequests.h"

#include "megaapi.h"
#include "mega/types.h"
#include "AvatarWidget.h"
#include "MegaApplication.h"

namespace UserAttributes
{
//FUL NAME REQUEST
//
//
void FullNameAttributeRequest::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
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

void FullNameAttributeRequest::requestAttribute()
{
    requestFirstNameAttribute();
    requestLastNameAttribute();
}

void FullNameAttributeRequest::updateAttributes(mega::MegaUser *user)
{
    bool hasFirstnameChanged = user->hasChanged(mega::MegaUser::CHANGE_TYPE_FIRSTNAME);
    bool hasLastnameChanged = user->hasChanged(mega::MegaUser::CHANGE_TYPE_LASTNAME);

    if(hasFirstnameChanged || hasLastnameChanged)
    {
        if (hasFirstnameChanged)
        {
            requestFirstNameAttribute();

        }

        if(hasLastnameChanged)
        {
            requestLastNameAttribute();
        }
    }
}

void FullNameAttributeRequest::requestFirstNameAttribute()
{
    mFirstName.clear();
    MegaSyncApp->getMegaApi()->getUserAttribute(mUserEmail.toUtf8().constData(),mega::ATTR_FIRSTNAME);
}

void FullNameAttributeRequest::requestLastNameAttribute()
{
    mLastName.clear();
    MegaSyncApp->getMegaApi()->getUserAttribute(mUserEmail.toUtf8().constData(),mega::ATTR_LASTNAME);
}


QString FullNameAttributeRequest::getFullName() const
{
    if(!isAttributeReady())
    {
        return getEmail();
    }

    return QString(QString::fromUtf8("%1 %2")).arg(mFirstName).arg(mLastName);
}

QString FullNameAttributeRequest::getRichFullName() const
{
    auto text = QString::fromUtf8("%1 %2").arg(mFirstName).arg(mLastName);

    text = text.replace(QString::fromUtf8("&"), QString::fromUtf8("&amp;"));
    text = text.replace(QString::fromUtf8("\""), QString::fromUtf8("&quot;"));
    text = text.replace(QString::fromUtf8("<"), QString::fromUtf8("&lt;"));
    text = text.replace(QString::fromUtf8(">"), QString::fromUtf8("&gt;"));

    return text;
}

bool FullNameAttributeRequest::isAttributeReady() const
{
    return !mFirstName.isEmpty() && !mLastName.isEmpty();
}

const QString &FullNameAttributeRequest::getFirstName() const
{
    return mFirstName;
}

const QString &FullNameAttributeRequest::getLastName() const
{
    return mLastName;
}

std::shared_ptr<const FullNameAttributeRequest> FullNameAttributeRequest::requestFullName(const char *user_email)
{
    if(user_email)
    {
        return UserAttributesManager::instance().requestAttribute<FullNameAttributeRequest>(user_email);
    }
    else
    {
        return nullptr;
    }
}


//AVATAR REQUEST
//
//
AvatarAttributeRequest::AvatarAttributeRequest(const QString &userEmail)
 : AttributeRequest(userEmail)
{
    mFullNameRequest = UserAttributesManager::instance().requestAttribute<FullNameAttributeRequest>(userEmail.toUtf8());
    connect(mFullNameRequest.get(), &FullNameAttributeRequest::attributeReady, this, &AvatarAttributeRequest::onFullNameAttributeReady);

    getLetterColor();
    fillLetterInfo();
}

std::shared_ptr<const AvatarAttributeRequest> AvatarAttributeRequest::requestAvatar(const char *user_email)
{
    return UserAttributesManager::instance().requestAttribute<AvatarAttributeRequest>(user_email);
}

std::shared_ptr<FullNameAttributeRequest> AvatarAttributeRequest::getFullNameRequest()
{
    return mFullNameRequest;
}

void AvatarAttributeRequest::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    if(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_AVATAR)
    {
        mIconPath = QString::fromUtf8(incoming_request->getFile());
        #ifdef WIN32
        if (mIconPath.startsWith(QString::fromUtf8("\\\\?\\")))
        {
            mIconPath = mIconPath.mid(4);
        }
        #endif

        QFile iconFile(mIconPath);

        if(!iconFile.exists() || e->getErrorCode() == mega::MegaError::API_ENOENT)
        {
            mIcon.clear();

            fillLetterInfo();
            getLetterColor();

            QFile::remove(mIconPath);
        }
        else
        {
            mLetterAvatarInfo.clear();
            mIcon.clear();
        }

        if(isAttributeReady())
        {
            emit attributeReady();
        }
    }
}

void AvatarAttributeRequest::requestAttribute()
{
    MegaSyncApp->getMegaApi()->getUserAvatar(mUserEmail.toUtf8(),
                                         Utilities::getAvatarPath(getEmail()).toUtf8());
}

void AvatarAttributeRequest::updateAttributes(mega::MegaUser *user)
{
    if (user->hasChanged(mega::MegaUser::CHANGE_TYPE_AVATAR))
    {
        requestAttribute();
    }
}

void AvatarAttributeRequest::onFullNameAttributeReady()
{
    if(isAttributeReady())
    {
        requestAttribute();
        emit attributeReady();
    }
}

void AvatarAttributeRequest::fillLetterInfo()
{
    if(mFullNameRequest && mFullNameRequest->isAttributeReady())
    {
        auto name = mFullNameRequest->getFirstName();
        if(!name.isEmpty())
        {
            mLetterAvatarInfo.letter = name.toUpper().at(0);
        }
    }

    if(mLetterAvatarInfo.isEmpty())
    {
        mLetterAvatarInfo.letter = getEmail().toUpper().at(0);
    }
}

void AvatarAttributeRequest::getLetterColor()
{
    const char* color = MegaSyncApp->getMegaApi()->getUserAvatarColor(MegaSyncApp->getMegaApi()->handleToBase64(qHash(getEmail())));
    mLetterAvatarInfo.color = color == nullptr ? Qt::red : QColor(color);

    delete [] color;
}

const QPixmap& AvatarAttributeRequest::getPixmap(const int& size) const
{
    auto& icon = mIcon[size];
    if(icon.isNull())
    {
        if(!mLetterAvatarInfo.isEmpty())
        {           
            icon = AvatarPixmap::createFromLetter(mLetterAvatarInfo.letter, mLetterAvatarInfo.color, size);
        }
        else
        {
            icon = AvatarPixmap::maskFromImagePath(mIconPath, size);
        }
    }

    return icon;
}

bool AvatarAttributeRequest::isAttributeReady() const
{
    return !mIconPath.isEmpty() && (mFullNameRequest && mFullNameRequest->isAttributeReady());
}

}//end namespace UserAttributes
