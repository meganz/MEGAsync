#include "UserAlert.h"

#include "EmailRequester.h"

UserAlert::UserAlert(mega::MegaUserAlert* megaUserAlert, QObject* parent)
    : UserMessage(UserMessage::Type::ALERT, parent)
    , mMegaUserAlert(megaUserAlert)
    , mAlertType(AlertType::UNKNOWN)
    , mEmail()
{
    init();
}

UserAlert::~UserAlert()
{
    mEmail.clear();
}

void UserAlert::init()
{
    assert(mMegaUserAlert != nullptr);

    if (mMegaUserAlert->getUserHandle() != mega::INVALID_HANDLE)
    {
        auto requestInfo = EmailRequester::getRequest(mMegaUserAlert->getUserHandle(), QString::fromUtf8(mMegaUserAlert->getEmail()));

        connect(requestInfo, &RequestInfo::emailChanged, this, &UserAlert::setEmail, Qt::QueuedConnection);
    }

    if (mMegaUserAlert->getEmail())
    {
        mEmail = QString::fromUtf8(mMegaUserAlert->getEmail());
    }
    else if (mMegaUserAlert->getUserHandle() != mega::INVALID_HANDLE)
    {
        mEmail = EmailRequester::instance()->getEmail(mMegaUserAlert->getUserHandle());
    }

    initAlertType();
}

void UserAlert::initAlertType()
{
    switch (mMegaUserAlert->getType())
    {
        case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
        case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
        case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
        {
            mAlertType = AlertType::ALERT_CONTACTS;
            break;
        }
        case mega::MegaUserAlert::TYPE_NEWSHARE:
        case mega::MegaUserAlert::TYPE_DELETEDSHARE:
        case mega::MegaUserAlert::TYPE_NEWSHAREDNODES:
        case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        case mega::MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            mAlertType = AlertType::ALERT_SHARES;
            break;
        }
        case mega::MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        case mega::MegaUserAlert::TYPE_PAYMENT_FAILED:
        case mega::MegaUserAlert::TYPE_PAYMENTREMINDER:
        {
            mAlertType = AlertType::ALERT_PAYMENTS;
            break;
        }
        case mega::MegaUserAlert::TYPE_TAKEDOWN:
        case mega::MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
        {
            mAlertType = AlertType::ALERT_TAKEDOWNS;
            break;
        }
        default:
        {
            // TODO: Show warning
            break;
        }
    }
}

UserAlert& UserAlert::operator=(UserAlert&& megaUserAlert)
{
    mMegaUserAlert.reset(megaUserAlert.mMegaUserAlert.release());
    megaUserAlert.mMegaUserAlert = nullptr;

    mEmail = megaUserAlert.mEmail;
    megaUserAlert.mEmail.clear();

    return *this;
}

QString UserAlert::getEmail() const
{
    return mEmail;
}

void UserAlert::setEmail(QString email)
{
    if (!email.isEmpty() && email != mEmail)
    {
        mEmail = email;

        emit emailChanged();
    }
}

bool UserAlert::isValid() const
{
    return mMegaUserAlert != nullptr;
}

void UserAlert::reset(mega::MegaUserAlert* alert)
{
    mMegaUserAlert.reset(alert);

    init();
}

unsigned int UserAlert::getId() const
{
    return mMegaUserAlert->getId();
}

bool UserAlert::isSeen() const
{
    return mMegaUserAlert->getSeen();
}

bool UserAlert::getRelevant() const
{
    return mMegaUserAlert->getRelevant();
}

int UserAlert::getType() const
{
    return mMegaUserAlert->getType();
}

mega::MegaHandle UserAlert::getUserHandle() const
{
    return mMegaUserAlert->getUserHandle();
}

int64_t UserAlert::getTimestamp(unsigned int index) const
{
    return mMegaUserAlert->getTimestamp(index);
}

int64_t UserAlert::getNumber(unsigned int index) const
{
    return mMegaUserAlert->getNumber(index);
}

mega::MegaHandle UserAlert::getNodeHandle() const
{
    return mMegaUserAlert->getNodeHandle();
}

const char* UserAlert::getString(unsigned int index) const
{
    return mMegaUserAlert->getString(index);
}

const char* UserAlert::getTitle() const
{
    return mMegaUserAlert->getTitle();
}

AlertType UserAlert::getAlertType() const
{
    return mAlertType;
}
