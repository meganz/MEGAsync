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
// - the first symbol of the user's name
// - the first letter of the user's email
//
// The flow is the following:
// 0. Use local cached avatar file if it exists
// 1. If not, request the user's avatar
// 2. When the Avatar request comes back:
// 2.a. If we have a picture, use it and emit attributeReady().
// 2.b. If not, request the full name
// 3. When the request for fullname comes back:
// 3.a. Update the symbol info with the first symbol from the user's first name
// 3.b. If we don't have the Name either, use the first letter from the user's email
// 3.c. Emit attributeReady()
//
// The avatar is updated on attribute change. If there is no picture, see above.
// If a user changes its avatar while the app is not running, we know about it
// with an action packet and we force the update.
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

//
// Returns true if the following conditions have been met:
//  1)  The hash for this file matches with the hash that was stored in the Preferences (.cfg)
//      This is for security reasons, to check that the file was not tempered with
//  2)  @filePath is not empty, the file exists and can be opened for reading
// Returns false otherwise
bool Avatar::isFileValid(const QString& filePath)
{
    QString stored_hash = Preferences::instance()->fileHash(filePath);
    if (stored_hash.isEmpty()) { return false; }

    QString new_hash = Utilities::getFileHash(filePath);
    return (!new_hash.isEmpty() && new_hash == stored_hash);
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

                QString new_hash = Utilities::getFileHash(mIconPath);

                if (!new_hash.isEmpty())
                {
                    // Store the hash, so next time we have a secure way of fetching the local avatar
                    Preferences::instance()->setFileHash(mIconPath, new_hash);
                }
            }
            if (mFullName)
            {
                disconnect(mFullName.get(), &FullName::fullNameReady, this, &Avatar::onFullNameAttributeReady);
            }
        }
        else
        {
            if(!mIconPath.isEmpty())
            {
                QFile::remove(mIconPath);
                mIconPath.clear();
            }
            mIcon.clear();
            if (!mFullName)
            {
                mFullName = FullName::requestFullName(getEmail().toUtf8().constData());
            }
            connect(mFullName.get(), &FullName::fullNameReady, this, &Avatar::onFullNameAttributeReady, Qt::UniqueConnection);
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
    QString email (getEmail());
    QString avatarPath (Utilities::getAvatarPath(email.isEmpty() ? Preferences::instance()->email() : email));

    std::function<void()> avatarRequestFunc = [this, email, avatarPath]()
    {
        mUseImgFile = false;
        mLetterAvatarInfo.symbol.clear();
        MegaSyncApp->getMegaApi()->getUserAvatar(email.isEmpty() ? nullptr : email.toUtf8().constData(),
                                                 avatarPath.toUtf8().constData());
    };
    QSharedPointer<ParamInfo> avatarParamInfo(new ParamInfo(avatarRequestFunc));
    // Attempt to use local cached avatar
    avatarParamInfo->mNeedsRetry = !isFileValid(avatarPath);

    ParamInfoMap paramInfo({{mega::MegaApi::USER_ATTR_AVATAR, avatarParamInfo}});
    RequestInfo ret(paramInfo, QMap<uint64_t, int>({{mega::MegaUser::CHANGE_TYPE_AVATAR,
                                                mega::MegaApi::USER_ATTR_AVATAR}}));
    return ret;
}

void Avatar::requestAttribute()
{
    mUseImgFile = false;
    mLetterAvatarInfo.symbol.clear();

    QString email (getEmail());
    QString avatarPath (Utilities::getAvatarPath(email.isEmpty() ? Preferences::instance()->email() : email));

    if (isFileValid(avatarPath))
    {
        // Get local avatar
        mIconPath = avatarPath;
        mUseImgFile = true;
        mIcon.clear();

        if (mFullName)
        {
            disconnect(mFullName.get(), &FullName::fullNameReady, this, &Avatar::onFullNameAttributeReady);
        }
    }
    else
    {
        // Fetch remote avatar
        requestUserAttribute(mega::MegaApi::USER_ATTR_AVATAR);
    }
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
        if(!api)
        {
            return;
        }
        auto avatarEmail (getEmail());

        mega::MegaHandle userHandle (mega::INVALID_HANDLE);

        if (avatarEmail.isEmpty() || avatarEmail == Preferences::instance()->email())
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

            // Check that the loaded image is valid. If not, force request the avatar
            if (icon.isNull())
            {
                forceRequestAttribute();
            }
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
