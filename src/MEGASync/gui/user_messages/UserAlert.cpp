#include "UserAlert.h"

#include "EmailRequester.h"
#include "MegaApplication.h"

#include <QDateTime>

UserAlert::UserAlert(mega::MegaUserAlert* megaUserAlert, QObject* parent)
    : UserMessage(megaUserAlert->getId(), UserMessage::Type::ALERT, parent)
    , mMegaUserAlert(megaUserAlert)
    , mMessageType(MessageType::UNKNOWN)
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

    auto userHandle(mMegaUserAlert->getUserHandle());

    if (mMegaUserAlert->getEmail())
    {
        mEmail = QString::fromUtf8(mMegaUserAlert->getEmail());
    }

    if (userHandle != mega::INVALID_HANDLE)
    {
        if (mEmail.isEmpty())
        {
            mEmail = EmailRequester::instance()->getEmail(mMegaUserAlert->getUserHandle());
        }

        auto requestInfo =
            EmailRequester::getRequest(mMegaUserAlert->getUserHandle(),
                                       QString::fromUtf8(mMegaUserAlert->getEmail()));

        connect(requestInfo,
                &RequestInfo::emailChanged,
                this,
                &UserAlert::setEmail,
                Qt::QueuedConnection);
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
            mMessageType = MessageType::ALERT_CONTACTS;
            break;
        }
        case mega::MegaUserAlert::TYPE_NEWSHARE:
        case mega::MegaUserAlert::TYPE_DELETEDSHARE:
        case mega::MegaUserAlert::TYPE_NEWSHAREDNODES:
        case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        case mega::MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            mMessageType = MessageType::ALERT_SHARES;
            break;
        }
        case mega::MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        case mega::MegaUserAlert::TYPE_PAYMENT_FAILED:
        case mega::MegaUserAlert::TYPE_PAYMENTREMINDER:
        {
            mMessageType = MessageType::ALERT_PAYMENTS;
            break;
        }
        case mega::MegaUserAlert::TYPE_TAKEDOWN:
        case mega::MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
        {
            mMessageType = MessageType::ALERT_TAKEDOWNS;
            break;
        }
        default:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                               QString::fromLatin1("Unknown alert type type: %d")
                                   .arg(mMegaUserAlert->getType()).toStdString().c_str());
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
    mId = mMegaUserAlert->getId();
    init();
    emit dataReset();
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

MessageType UserAlert::getMessageType() const
{
    return mMessageType;
}

std::shared_ptr<mega::MegaNode> UserAlert::getAlertNode() const
{
    return std::shared_ptr<mega::MegaNode>(
        MegaSyncApp->getMegaApi()->getNodeByHandle(mMegaUserAlert->getNodeHandle()));
}

bool UserAlert::sort(UserMessage* checkWith) const
{
    if(auto checkAlert = dynamic_cast<UserAlert*>(checkWith))
    {
        auto thisDate = QDateTime::fromMSecsSinceEpoch(getTimestamp(0) * 1000);
        auto checkDate = QDateTime::fromMSecsSinceEpoch(checkAlert->getTimestamp(0) * 1000);
        return thisDate > checkDate;
    }

    return UserMessage::sort(checkWith);
}

bool UserAlert::isRowAccepted(MessageType type) const
{
    return type == getMessageType();
}
