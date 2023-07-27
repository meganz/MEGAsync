#include "PasswordStrengthChecker.h"
#include "MegaApplication.h"

PasswordStrengthChecker::PasswordStrengthChecker(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
{

}

PasswordStrengthChecker::PasswordStrength PasswordStrengthChecker::getPasswordStrength(const QString &password)
{
    return static_cast<PasswordStrength>(mMegaApi->getPasswordStrength(password.toUtf8().constData()));
}
