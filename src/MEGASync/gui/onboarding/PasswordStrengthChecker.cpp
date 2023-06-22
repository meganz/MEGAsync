#include "PasswordStrengthChecker.h"
#include "MegaApplication.h"

PasswordStrengthChecker::PasswordStrengthChecker()
    : mMegaApi(MegaSyncApp->getMegaApi())
{

}

PasswordStrengthChecker::PasswordStrength PasswordStrengthChecker::getPasswordStrength(const QString &password)
{
    return static_cast<PasswordStrength>(mMegaApi->getPasswordStrength(password.toUtf8().constData()));
}
