#include "MegaUserAlertExt.h"

#include "EmailRequester.h"

MegaUserAlertExt::MegaUserAlertExt(mega::MegaUserAlert* megaUserAlert, QObject *parent)
    : QObject(parent),
    mMegaUserAlert(megaUserAlert)
{
    init();
}

MegaUserAlertExt::~MegaUserAlertExt()
{
    mEmail.clear();
}

void MegaUserAlertExt::init()
{
    assert(mMegaUserAlert != nullptr);

    connect(EmailRequester::instance(), &EmailRequester::emailChanged, this, &MegaUserAlertExt::setEmail, Qt::QueuedConnection);

    if (mMegaUserAlert->getUserHandle() != mega::INVALID_HANDLE)
    {
        EmailRequester::instance()->addEmailTracking(mMegaUserAlert->getUserHandle());
    }

    if (mMegaUserAlert->getEmail())
    {
        mEmail = QString::fromUtf8(mMegaUserAlert->getEmail());
    }
    else if (mMegaUserAlert->getUserHandle() != mega::INVALID_HANDLE)
    {
        auto email = EmailRequester::instance()->getEmail(mMegaUserAlert->getUserHandle(), true);
        if (!email.isEmpty())
        {
            setEmail();
        }
    }
}

MegaUserAlertExt& MegaUserAlertExt::operator=(MegaUserAlertExt&& megaUserAlert)
{
    mMegaUserAlert.reset(megaUserAlert.mMegaUserAlert.release());
    megaUserAlert.mMegaUserAlert = nullptr;

    mEmail = megaUserAlert.mEmail;
    megaUserAlert.mEmail.clear();

    return *this;
}

QString MegaUserAlertExt::getEmail() const
{
    return mEmail;
}

void MegaUserAlertExt::setEmail()
{
    if (mMegaUserAlert->getUserHandle() == mega::INVALID_HANDLE)
    {
        return;
    }

    QString email = EmailRequester::instance()->getEmail(mMegaUserAlert->getUserHandle(), false);
    if (!email.isEmpty() && email != mEmail)
    {
        mEmail = email;

        emit emailChanged();
    }
}

bool MegaUserAlertExt::isValid() const
{
    return mMegaUserAlert != nullptr;
}

void MegaUserAlertExt::reset(mega::MegaUserAlert* alert)
{
    mMegaUserAlert.reset(alert);

    init();
}

unsigned int MegaUserAlertExt::getId() const
{
    return mMegaUserAlert->getId();
}

bool MegaUserAlertExt::getSeen() const
{
    return mMegaUserAlert->getSeen();
}

bool MegaUserAlertExt::getRelevant() const
{
    return mMegaUserAlert->getRelevant();
}

int MegaUserAlertExt::getType() const
{
    return mMegaUserAlert->getType();
}

mega::MegaHandle MegaUserAlertExt::getUserHandle() const
{
    return mMegaUserAlert->getUserHandle();
}

int64_t MegaUserAlertExt::getTimestamp(unsigned int index) const
{
    return mMegaUserAlert->getTimestamp(index);
}

int64_t MegaUserAlertExt::getNumber(unsigned int index) const
{
    return mMegaUserAlert->getNumber(index);
}

mega::MegaHandle MegaUserAlertExt::getNodeHandle() const
{
    return mMegaUserAlert->getNodeHandle();
}

const char* MegaUserAlertExt::getString(unsigned int index) const
{
    return mMegaUserAlert->getString(index);
}

const char* MegaUserAlertExt::getTitle() const
{
    return mMegaUserAlert->getTitle();
}
