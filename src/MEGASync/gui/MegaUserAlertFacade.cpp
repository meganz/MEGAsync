#include "MegaUserAlertFacade.h"

MegaUserAlertFacade::MegaUserAlertFacade(mega::MegaUserAlert* megaUserAlert, QObject *parent)
    : QObject{parent},
    MegaUserAlert(*megaUserAlert),
    mMegaUserAlert(megaUserAlert)
{

}

const char* MegaUserAlertFacade::getEmail() const
{
    if (mEmail.empty())
    {
        return MegaUserAlert::getEmail();
    }
    else
    {
        return mEmail.c_str();
    }
}

void MegaUserAlertFacade::setEmail(QString email)
{
    auto toStoreEmail = email.toStdString();

    if (toStoreEmail != mEmail)
    {
        mEmail = toStoreEmail;

        emit emailChanged();
    }
}

bool MegaUserAlertFacade::isValid() const
{
    return mMegaUserAlert.get() != nullptr;
}

void MegaUserAlertFacade::reset(MegaUserAlert* alert)
{
    mMegaUserAlert.reset(alert);
}
