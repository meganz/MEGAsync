#ifndef BUTTONTOKENSBYTYPE_H
#define BUTTONTOKENSBYTYPE_H

#include <QAbstractButton>

class ButtonTokensByType
{
public:
    static void setDefaultTokens(QAbstractButton* button);

private:
    static bool propertyEmpty(QAbstractButton* button, const char* propertyName);
};

#endif // BUTTONTOKENSBYTYPE_H
