#ifndef PASSWORDSTRENGTHCHECKER_H
#define PASSWORDSTRENGTHCHECKER_H

#include "megaapi.h"

#include <QObject>

class PasswordStrengthChecker : public QObject
{
    Q_OBJECT

    enum PasswordStrength {
        PasswordStrengthVeryWeak,
        PasswordStrengthWeak,
        PasswordStrengthMedium,
        PasswordStrengthGood,
        PasswordStrengthStrong
    };

public:
    PasswordStrengthChecker(QObject* parent = nullptr);

    Q_INVOKABLE PasswordStrength getPasswordStrength(const QString& password);

private:
    mega::MegaApi* mMegaApi;
};

#endif // PASSWORDSTRENGTHCHECKER_H
