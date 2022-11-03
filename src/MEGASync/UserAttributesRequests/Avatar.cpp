#include "Avatar.h"
#include "FullName.h"
#include "megaapi.h"
#include "mega/types.h"
#include "AvatarWidget.h"
#include "MegaApplication.h"
#include "Preferences.h"

namespace UserAttributes
{
// AVATAR REQUEST
//
// The Avatar is either (in order of preference)
// - an image set by the user
// - the first letter of the user's name
// - the first letter of the user's email
//
// The flow is the following:
// 1. Request the user's avatar
// 2. When the Avatar request comes back:
// 2.a. If we have a picture, use it and emit attributeReady().
// 2.b. If not, request the full name
// 3. When the request for fullname comes back:
// 3.a. Update the symbol info with the fisrt symbol from the user's first name
// 3.b. If we don't have the Name either, use the first letter from the user's email
// 3.c. Emit attributeReady()
//
// The avatar is updated on attribute change. If there is no picture, see above.
//

Avatar::Avatar(const QString &userEmail)
 : AttributeRequest(userEmail), mUseImgFile(true)
{
    mLetterAvatarInfo.clear();
}

std::shared_ptr<const Avatar> Avatar::requestAvatar(const char *user_email)
{
    return UserAttributesManager::instance().requestAttribute<Avatar>(user_email);
}

void Avatar::onRequestFinish(mega::MegaApi*, mega::MegaRequest* incoming_request, mega::MegaError* e)
{
    if(incoming_request->getParamType() == mega::MegaApi::USER_ATTR_AVATAR)
    {
        mUseImgFile = false;
        auto errorCode (e->getErrorCode());

        if (errorCode == mega::MegaError::API_OK)
        {
            mIconPath = QString::fromUtf8(incoming_request->getFile());
            #ifdef WIN32
            if (mIconPath.startsWith(QString::fromUtf8("\\\\?\\")))
            {
                mIconPath = mIconPath.mid(4);
            }
            #endif
            if (QFile::exists(mIconPath))
            {
                mUseImgFile = true;
                mIcon.clear();
            }
            if (mFullName)
            {
                disconnect(mFullName.get(), &FullName::attributeReady, this, &Avatar::onFullNameAttributeReady);
            }
        }
        else
        {
            QFile::remove(mIconPath);
            mIconPath.clear();
            mIcon.clear();
            if (!mFullName)
            {
                mFullName = FullName::requestFullName(getEmail().toUtf8().constData());
            }
            connect(mFullName.get(), &FullName::attributeReady, this, &Avatar::onFullNameAttributeReady);
            onFullNameAttributeReady();
        }

        if (isAttributeReady())
        {
            emit attributeReady();
        }
    }
}

AttributeRequest::RequestInfo Avatar::fillRequestInfo()
{
    std::function<void()> avatarRequestFunc = [this]()
    {
        mUseImgFile = false;
        mLetterAvatarInfo.symbol.clear();

        QString email (getEmail());
        QString avatarPath (Utilities::getAvatarPath(email.isEmpty() ? Preferences::instance()->email() : email));
        MegaSyncApp->getMegaApi()->getUserAvatar(email.isEmpty() ? nullptr : email.toUtf8().constData(),
                                                 avatarPath.toUtf8().constData());
    };
    QSharedPointer<ParamInfo> avatarParamInfo(new ParamInfo(avatarRequestFunc, QList<int>()
                                                            << mega::MegaError::API_OK
                                                            << mega::MegaError::API_ENOENT));
    ParamInfoMap paramInfo({{mega::MegaApi::USER_ATTR_AVATAR, avatarParamInfo}});
    RequestInfo ret(paramInfo, QMap<int, int>({{mega::MegaUser::CHANGE_TYPE_AVATAR,
                                                mega::MegaApi::USER_ATTR_AVATAR}}));
    return ret;
}

void Avatar::requestAttribute()
{
    requestUserAttribute(mega::MegaApi::USER_ATTR_AVATAR);
}

void Avatar::onFullNameAttributeReady()
{
    // This case handles the situation when the name changes and the user does not
    // have an avatar set. Only emit if the symbol changes.
    auto oldSymbol (mLetterAvatarInfo.symbol);
    fillLetterInfo();
    if (!mUseImgFile && oldSymbol != mLetterAvatarInfo.symbol)
    {
        mIcon.clear();
        emit attributeReady();
    }
}

void Avatar::fillLetterInfo()
{
    if (mFullName && !mFullName->isRequestPending())
    {
        getLetterColor();
        auto name = mFullName->getFullName();
        if(!name.isEmpty())
        {
            auto c (name.cbegin());
            mLetterAvatarInfo.symbol = c->toUpper();
            if (c->isHighSurrogate() && ++c != name.cend() && c->isLowSurrogate())
            {
                mLetterAvatarInfo.symbol += *c;
            }
        }
    }
}

void Avatar::getLetterColor()
{
    if (mLetterAvatarInfo.colorNeedsRefresh)
    {
        auto api = MegaSyncApp->getMegaApi();
        auto avatarEmail (getEmail());

        mega::MegaHandle userHandle (mega::INVALID_HANDLE);

        if (avatarEmail.isEmpty())
        {
            userHandle = api->getMyUserHandleBinary();
        }
        else
        {
            std::unique_ptr<mega::MegaUser> user (api->getContact(avatarEmail.toUtf8().constData()));
            if (user)
            {
                userHandle = user->getHandle();
            }
            else
            {
                std::unique_ptr<char[]> loggedUserEmailStr (api->getMyEmail());
                auto loggedUserEmail (QString::fromUtf8(loggedUserEmailStr.get()));
                if (avatarEmail == loggedUserEmail)
                {
                    userHandle = api->getMyUserHandleBinary();
                }
            }
        }
        std::unique_ptr<char[]> userHandleStr (api->userHandleToBase64(userHandle));
        std::unique_ptr<char[]> primaryColor (api->getUserAvatarColor(userHandleStr.get()));
        mLetterAvatarInfo.primaryColor = QColor(primaryColor.get());
        std::unique_ptr<char[]> secondaryColor (api->getUserAvatarSecondaryColor(userHandleStr.get()));
        mLetterAvatarInfo.secondaryColor = QColor(secondaryColor.get());

        mLetterAvatarInfo.colorNeedsRefresh = userHandle != mega::INVALID_HANDLE;
    }
}

const QPixmap& Avatar::getPixmap(const int& size) const
{
    auto& icon = mIcon[size];
    if(icon.isNull())
    {
        if(!mUseImgFile)
        {
            // If the attribute is not ready, use the first char of the email as a placeholder.
            icon = AvatarPixmap::createFromLetter(isAttributeReady() ? mLetterAvatarInfo.symbol
                    : getEmail().at(0).toUpper(), mLetterAvatarInfo.primaryColor,
                                                  mLetterAvatarInfo.secondaryColor, size);
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
    // We need to have at least 1 valid avatar
    return !mLetterAvatarInfo.isEmpty() || !mIconPath.isEmpty();
}

}//end namespace UserAttributes
