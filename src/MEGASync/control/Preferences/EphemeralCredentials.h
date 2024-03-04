#ifndef EPHEMERALCREDENTIALS_H
#define EPHEMERALCREDENTIALS_H

#include <QString>
#include <QtCore/QSettings>
#include <QtCore/QDataStream>

class EphemeralCredentials
{
public:
    EphemeralCredentials() = default;
    EphemeralCredentials(const EphemeralCredentials& cred);
    EphemeralCredentials(EphemeralCredentials&& cred);
    EphemeralCredentials& operator=(const EphemeralCredentials& cred);
    EphemeralCredentials& operator=(EphemeralCredentials&& cred);

    QString email;
    QString sessionId;

    bool operator==(const EphemeralCredentials& cred) const;
};

QDataStream& operator>>(QDataStream& in, EphemeralCredentials& cred);
QDataStream& operator<<(QDataStream& out, const EphemeralCredentials& cred);

#endif // EPHEMERALCREDENTIALS_H
