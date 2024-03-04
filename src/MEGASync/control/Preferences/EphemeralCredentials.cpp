#include "EphemeralCredentials.h"

EphemeralCredentials::EphemeralCredentials(const EphemeralCredentials& cred)
    : email(cred.email)
    , sessionId(cred.sessionId)
{
}

EphemeralCredentials::EphemeralCredentials(EphemeralCredentials&& cred)
    : email(std::move(cred.email))
    , sessionId(std::move(cred.sessionId))
{
}

bool EphemeralCredentials::operator==(const EphemeralCredentials& cred) const
{
    return this->email == cred.email && this->sessionId == cred.sessionId;
}

EphemeralCredentials& EphemeralCredentials::operator=(const EphemeralCredentials& cred)
{
    email = cred.email;
    sessionId = cred.sessionId;

    return *this;
}

EphemeralCredentials& EphemeralCredentials::operator=(EphemeralCredentials&& cred)
{
    email = std::move(cred.email);
    sessionId = std::move(cred.sessionId);

    return *this;
}

QDataStream& operator<<(QDataStream& out, const EphemeralCredentials& cred)
{
    out << cred.email << cred.sessionId;
    return out;
}

QDataStream& operator>>(QDataStream& in, EphemeralCredentials& cred)
{
    in >> cred.email;
    in >> cred.sessionId;
    return in;
}

