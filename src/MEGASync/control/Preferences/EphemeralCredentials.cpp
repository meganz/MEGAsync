#include "EphemeralCredentials.h"

EphemeralCredentials::EphemeralCredentials()
    : email(QString())
    , password(QString())
    , sessionId(QString())
{
}

EphemeralCredentials::EphemeralCredentials(const EphemeralCredentials &cred)
{
    this->email = cred.email;
    this->password = cred.password;
    this->sessionId = cred.sessionId;
}

bool EphemeralCredentials::operator==(const EphemeralCredentials& cred) const
{
    return this->email == cred.email && this->sessionId == cred.sessionId
           && this->password == cred.password;
}


QDataStream& operator<<(QDataStream& out, const EphemeralCredentials& cred)
{
    out<<cred.email<<cred.sessionId<<cred.password;
    return out;
}

QDataStream& operator>>(QDataStream& in, EphemeralCredentials& cred)
{
    in >> cred.email;
    in >> cred.sessionId;
    in >> cred.password;
    return in;
}

