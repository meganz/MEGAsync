#include "TokenPropertySetter.h"

#include "TokenPropertyNames.h"

TokenPropertySetter::TokenPropertySetter(const BaseTokens& baseTokens,
                                         const ButtonTokens& buttonTokens):
    mBaseTokens(baseTokens),
    mButtonTokens(buttonTokens)
{}

void TokenPropertySetter::applyTokens(QWidget* target)
{
    // Base tokens
    applyProperty(TOKEN_PROPERTIES::normalOff, mBaseTokens.getNormalOffToken(), target);
    applyProperty(TOKEN_PROPERTIES::normalOn, mBaseTokens.getNormalOnToken(), target);
    applyProperty(TOKEN_PROPERTIES::disabledOff, mBaseTokens.getDisabledOffToken(), target);
    applyProperty(TOKEN_PROPERTIES::disabledOn, mBaseTokens.getDisabledOnToken(), target);

    // Button tokens
    applyProperty(TOKEN_PROPERTIES::hoverOff, mButtonTokens.getHoverOffToken(), target);
    applyProperty(TOKEN_PROPERTIES::hoverOn, mButtonTokens.getHoverOnToken(), target);
    applyProperty(TOKEN_PROPERTIES::pressedOff, mButtonTokens.getPressedOffToken(), target);
    applyProperty(TOKEN_PROPERTIES::pressedOn, mButtonTokens.getPressedOnToken(), target);
}

void TokenPropertySetter::applyProperty(const char* property, const QString& value, QWidget* target)
{
    if (!value.isEmpty())
    {
        target->setProperty(property, value);
    }
}
