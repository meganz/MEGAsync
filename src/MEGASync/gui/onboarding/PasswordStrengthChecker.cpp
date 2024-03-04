#include "PasswordStrengthChecker.h"
#include "MegaApplication.h"

PasswordStrengthChecker::PasswordStrengthChecker(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mPassword(QString())
    , mStrength(PASSWORD_STRENGTH_VERYWEAK)
{
}

void PasswordStrengthChecker::setPassword(const QString& password)
{
    mPassword = password;
    PasswordStrength newStrength =
        static_cast<PasswordStrength>(mMegaApi->getPasswordStrength(mPassword.toUtf8().constData()));
    if(mStrength != newStrength)
    {
        mStrength = newStrength;
        emit strengthChanged();
    }
}

PasswordStrengthChecker::PasswordStrength PasswordStrengthChecker::getStrength() const
{
    return mStrength;
}
