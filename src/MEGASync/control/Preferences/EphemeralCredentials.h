#ifndef EPHEMERALCREDENTIALS_H
#define EPHEMERALCREDENTIALS_H

#include <QString>
#include <QtCore/QSettings>
#include <QtCore/QDataStream>

class EphemeralCredentials
{
public:
    EphemeralCredentials();
    EphemeralCredentials(const EphemeralCredentials& cred);

    QString email;
    QString sessionId;
    QString password;

    bool operator==(const EphemeralCredentials& cred) const;
    friend QDataStream& operator>>(QDataStream& in, EphemeralCredentials& cred);
    friend QDataStream& operator<<(QDataStream& out, const EphemeralCredentials& cred);
};

Q_DECLARE_METATYPE(EphemeralCredentials)

#endif // EPHEMERALCREDENTIALS_H
