#include "MegaUserAlertExt.h"

#include "EmailRequester.h"

/*
namespace
{
const std::map<AlertType, std::vector<int>> MegaUserAlertsByType
{
    {
        AlertType::CONTACTS,
        {
            mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST,
            mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED,
            mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED
        }
    },
    {
        AlertType::SHARES,
        {
            mega::MegaUserAlert::TYPE_NEWSHARE,
            mega::MegaUserAlert::TYPE_DELETEDSHARE,
            mega::MegaUserAlert::TYPE_NEWSHAREDNODES,
            mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES,
            mega::MegaUserAlert::TYPE_UPDATEDSHAREDNODES
        }
    },
    {
        AlertType::PAYMENTS,
        {
            mega::MegaUserAlert::TYPE_PAYMENT_SUCCEEDED,
            mega::MegaUserAlert::TYPE_PAYMENT_FAILED,
            mega::MegaUserAlert::TYPE_PAYMENTREMINDER
        }
    },
    {
        AlertType::TAKEDOWNS,
        {
            mega::MegaUserAlert::TYPE_TAKEDOWN,
            mega::MegaUserAlert::TYPE_TAKEDOWN_REINSTATED
        }
    }
};
}
*/

MegaUserAlertExt::MegaUserAlertExt(mega::MegaUserAlert* megaUserAlert, QObject* parent)
    : NotificationExtBase(NotificationExtBase::Type::ALERT, parent)
    , mMegaUserAlert(megaUserAlert)
    , mAlertType(AlertType::UNKNOWN)
    , mEmail()
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

    if (mMegaUserAlert->getUserHandle() != mega::INVALID_HANDLE)
    {
        auto requestInfo = EmailRequester::getRequest(mMegaUserAlert->getUserHandle(), QString::fromUtf8(mMegaUserAlert->getEmail()));

        connect(requestInfo, &RequestInfo::emailChanged, this, &MegaUserAlertExt::setEmail, Qt::QueuedConnection);
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

void MegaUserAlertExt::initAlertType()
{
    /*
    for (auto rit = MegaUserAlertsByType.rbegin(); rit != MegaUserAlertsByType.rend(); ++rit)
    {
        const auto& alertType = rit->first;
        const auto& alerts = rit->second;
        if (std::find(alerts.begin(), alerts.end(), mMegaUserAlert->getType()) != alerts.end())
        {
            mAlertType = alertType;
            return;
        }
    }

    if(mAlertType == AlertType::UNKNOWN)
    {
        // TODO: Show warning
    }
    */
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

void MegaUserAlertExt::setEmail(QString email)
{
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

bool MegaUserAlertExt::isSeen() const
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

AlertType MegaUserAlertExt::getAlertType() const
{
    return mAlertType;
}
