#ifndef PASSWORDSTRENGTHCHECKER_H
#define PASSWORDSTRENGTHCHECKER_H

#include "megaapi.h"

#include <QObject>

class PasswordStrengthChecker : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString password MEMBER mPassword WRITE setPassword)
    Q_PROPERTY(PasswordStrength strength MEMBER mStrength READ getStrength NOTIFY strengthChanged)

public:
    enum PasswordStrength {
        PASSWORD_STRENGTH_VERYWEAK = 0,
        PASSWORD_STRENGTH_WEAK = 1,
        PASSWORD_STRENGTH_MEDIUM = 2,
        PASSWORD_STRENGTH_GOOD = 3,
        PASSWORD_STRENGTH_STRONG = 4
    };
    Q_ENUM(PasswordStrength)

    PasswordStrengthChecker(QObject* parent = nullptr);

    void setPassword(const QString& password);
    PasswordStrength getStrength() const;

signals:
    void strengthChanged();

private:
    mega::MegaApi* mMegaApi;
    QString mPassword;
    PasswordStrength mStrength;

};

#endif // PASSWORDSTRENGTHCHECKER_H
