#ifndef PASSWORDSTRENGTHCHECKER_H
#define PASSWORDSTRENGTHCHECKER_H

#include "megaapi.h"

#include <QObject>

class PasswordStrengthChecker : public QObject
{
    Q_OBJECT

public:
    enum PasswordStrength {
        PasswordStrengthVeryWeak,
        PasswordStrengthWeak,
        PasswordStrengthMedium,
        PasswordStrengthGood,
        PasswordStrengthStrong
    };
    Q_ENUM(PasswordStrength)

    PasswordStrengthChecker(QObject* parent = nullptr);

    Q_INVOKABLE PasswordStrength getPasswordStrength(const QString& password);

private:
    mega::MegaApi* mMegaApi;
};

#endif // PASSWORDSTRENGTHCHECKER_H
