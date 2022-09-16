#include "Avatar.h"
#include "FullName.h"
#include "megaapi.h"
#include "mega/types.h"
#include "AvatarWidget.h"
#include "MegaApplication.h"

namespace UserAttributes
{
//AVATAR REQUEST
//
//
Avatar::Avatar(const QString &userEmail)
 : AttributeRequest(userEmail)
{
    mFullName = UserAttributesManager::instance().requestAttribute<FullName>(userEmail.toUtf8());
    connect(mFullName.get(), &FullName::attributeReady, this, &Avatar::onFullNameAttributeReady);

    getLetterColor();
    fillLetterInfo();
}

std::shared_ptr<const Avatar> Avatar::requestAvatar(const char *user_email)
{
    return UserAttributesManager::instance().requestAttribute<Avatar>(user_email);
}

std::shared_ptr<FullName> Avatar::getFullName()
{
    return mFullName;
}

void Avatar::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
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

void Avatar::requestAttribute()
{
    MegaSyncApp->getMegaApi()->getUserAvatar(mUserEmail.toUtf8(),
                                         Utilities::getAvatarPath(getEmail()).toUtf8());
}

void Avatar::updateAttributes(mega::MegaUser *user)
{
    if (user->hasChanged(mega::MegaUser::CHANGE_TYPE_AVATAR))
    {
        requestAttribute();
    }
}

void Avatar::onFullNameAttributeReady()
{
    if(isAttributeReady())
    {
        requestAttribute();
        emit attributeReady();
    }
}

void Avatar::fillLetterInfo()
{
    if (mFullName)
    {
        auto name = mFullName->getFullName();
        if(!name.isEmpty())
        {
            auto c (name.cbegin());
            mLetterAvatarInfo.letter = c->toUpper();
            if (c->isHighSurrogate() && ++c != name.cend() && c->isLowSurrogate())
            {
                mLetterAvatarInfo.letter += *c;
            }
        }
    }

    if(mLetterAvatarInfo.isEmpty())
    {
        mLetterAvatarInfo.letter = getEmail().at(0).toUpper();
    }
}

void Avatar::getLetterColor()
{
    const char* color = MegaSyncApp->getMegaApi()->getUserAvatarColor(MegaSyncApp->getMegaApi()->handleToBase64(qHash(getEmail())));
    mLetterAvatarInfo.color = color == nullptr ? Qt::red : QColor(color);

    delete [] color;
}

const QPixmap& Avatar::getPixmap(const int& size) const
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

bool Avatar::isAttributeReady() const
{
    return !mIconPath.isEmpty() && (mFullName && mFullName->isAttributeReady());
}

}//end namespace UserAttributes
