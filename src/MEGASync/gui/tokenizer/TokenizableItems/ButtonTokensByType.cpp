#include "ButtonTokensByType.h"

#include "TokenPropertyNames.h"

#include <QVariant>

void ButtonTokensByType::setDefaultTokens(QAbstractButton* button)
{
    auto buttonType = button->property("type").toString();

    if (buttonType == QLatin1String("primary"))
    {
        if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
        {
            button->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("text-inverse-accent"));
        }
    }
    else if (buttonType == QLatin1String("secondary"))
    {
        if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
        {
            button->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("icon-secondary"));
        }
    }
    else if (buttonType == QLatin1String("outline") || buttonType == QLatin1String("ghost"))
    {
        if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
        {
            button->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("button-outline"));
        }

        if (propertyEmpty(button, TOKEN_PROPERTIES::hoverOff))
        {
            button->setProperty(TOKEN_PROPERTIES::hoverOff, QLatin1String("button-outline-hover"));
        }

        if (propertyEmpty(button, TOKEN_PROPERTIES::pressedOff))
        {
            button->setProperty(TOKEN_PROPERTIES::pressedOff,
                                QLatin1String("button-outline-pressed"));
        }
    }
    else if (buttonType == QLatin1String("link"))
    {
        if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
        {
            button->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("link-primary"));
        }

        if (propertyEmpty(button, TOKEN_PROPERTIES::disabledOff))
        {
            button->setProperty(TOKEN_PROPERTIES::disabledOff, QLatin1String("button-disabled"));
        }
    }

    if (propertyEmpty(button, TOKEN_PROPERTIES::disabledOff))
    {
        button->setProperty(TOKEN_PROPERTIES::disabledOff, QLatin1String("button-disabled"));
    }
}

bool ButtonTokensByType::propertyEmpty(QAbstractButton* button, const char* propertyName)
{
    return !button->property(propertyName).isValid();
}
