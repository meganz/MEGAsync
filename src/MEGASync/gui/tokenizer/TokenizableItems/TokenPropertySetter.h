#ifndef TOKENPROPERTYSETTER_H
#define TOKENPROPERTYSETTER_H

#include "TokenizableButtons.h"

class TokenPropertySetter
{
public:
    TokenPropertySetter(const BaseTokens& baseTokens,
                        const ButtonTokens& buttonTokens = ButtonTokens());

    void applyTokens(QWidget* target);

private:
    void applyProperty(const char* property, const QString& value, QWidget* target);

    BaseTokens mBaseTokens;
    ButtonTokens mButtonTokens;
};

#endif // TOKENPROPERTYSETTER_H
