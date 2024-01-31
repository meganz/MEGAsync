#include "MegaUserAlertExt.h"

MegaUserAlertExt::MegaUserAlertExt(mega::MegaUserAlert* megaUserAlert, QObject *parent)
    : QObject{parent},
    mMegaUserAlert(megaUserAlert)
{}

const char* MegaUserAlertExt::getEmail() const
{
    if (mEmail.empty() && mMegaUserAlert != nullptr)
    {
        return mMegaUserAlert->getEmail();
    }
    else
    {
        return mEmail.c_str();
    }
}

void MegaUserAlertExt::setEmail(QString email)
{
    auto toStoreEmail = email.toStdString();

    if (toStoreEmail != mEmail)
    {
        mEmail = toStoreEmail;

        emit emailChanged();
    }
}

bool MegaUserAlertExt::isValid() const
{
    return mMegaUserAlert != nullptr;
}

void MegaUserAlertExt::reset(mega::MegaUserAlert* alert)
{
    //mMegaUserAlert.reset(alert);
    mMegaUserAlert = alert;
}

unsigned int MegaUserAlertExt::getId() const
{
    return mMegaUserAlert->getId();
}

bool MegaUserAlertExt::getSeen() const
{
    return mMegaUserAlert->getSeen();
}

int MegaUserAlertExt::getType() const
{
    return mMegaUserAlert->getType();
}

int64_t MegaUserAlertExt::getTimestamp(unsigned int index) const
{
    return mMegaUserAlert->getTimestamp(index);
}
