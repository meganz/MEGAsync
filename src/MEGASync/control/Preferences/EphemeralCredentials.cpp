#include "EphemeralCredentials.h"

EphemeralCredentials::EphemeralCredentials(const EphemeralCredentials &cred)
    : email(cred.email)
    , sessionId(cred.sessionId)
{
}

bool EphemeralCredentials::operator==(const EphemeralCredentials& cred) const
{
    return this->email == cred.email && this->sessionId == cred.sessionId;
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

