#include "ButtonTokensByType.h"

#include "TokenPropertyNames.h"

#include <QVariant>

void ButtonTokensByType::setDefaultTokens(QAbstractButton* button)
{
    auto buttonType = button->property("type").toString();
    auto isCheckable(button->isCheckable());

    if (buttonType == QLatin1String("primary") || buttonType == QLatin1String("secondary"))
    {
        if (buttonType == QLatin1String("primary"))
        {
            if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOff,
                                    QLatin1String("text-inverse-accent"));
            }

            if (isCheckable && propertyEmpty(button, TOKEN_PROPERTIES::normalOn))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOn,
                                    QLatin1String("text-inverse-accent"));
            }
        }
        else if (buttonType == QLatin1String("secondary"))
        {
            if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("icon-secondary"));
            }

            if (isCheckable && propertyEmpty(button, TOKEN_PROPERTIES::normalOn))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOn, QLatin1String("icon-secondary"));
            }
        }

        if (propertyEmpty(button, TOKEN_PROPERTIES::disabledOff))
        {
            button->setProperty(TOKEN_PROPERTIES::disabledOff, QLatin1String("icon-disabled"));
        }
    }
    else
    {
        if (buttonType == QLatin1String("outline") || buttonType == QLatin1String("ghost"))
        {
            if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("button-outline"));
            }

            if (isCheckable && propertyEmpty(button, TOKEN_PROPERTIES::normalOn))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOn, QLatin1String("button-outline"));
            }

            if (propertyEmpty(button, TOKEN_PROPERTIES::hoverOff))
            {
                button->setProperty(TOKEN_PROPERTIES::hoverOff,
                                    QLatin1String("button-outline-hover"));
            }

            if (isCheckable && propertyEmpty(button, TOKEN_PROPERTIES::hoverOn))
            {
                button->setProperty(TOKEN_PROPERTIES::hoverOn,
                                    QLatin1String("button-outline-hover"));
            }

            if (propertyEmpty(button, TOKEN_PROPERTIES::pressedOff))
            {
                button->setProperty(TOKEN_PROPERTIES::pressedOff,
                                    QLatin1String("button-outline-pressed"));
            }

            if (isCheckable && propertyEmpty(button, TOKEN_PROPERTIES::pressedOn))
            {
                button->setProperty(TOKEN_PROPERTIES::pressedOn,
                                    QLatin1String("button-outline-pressed"));
            }
        }
        else if (buttonType == QLatin1String("link"))
        {
            if (propertyEmpty(button, TOKEN_PROPERTIES::normalOff))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("link-primary"));
            }

            if (isCheckable && propertyEmpty(button, TOKEN_PROPERTIES::normalOn))
            {
                button->setProperty(TOKEN_PROPERTIES::normalOn, QLatin1String("link-primary"));
            }

            if (propertyEmpty(button, TOKEN_PROPERTIES::disabledOff))
            {
                button->setProperty(TOKEN_PROPERTIES::disabledOff,
                                    QLatin1String("button-disabled"));
            }

            if (isCheckable && propertyEmpty(button, TOKEN_PROPERTIES::disabledOn))
            {
                button->setProperty(TOKEN_PROPERTIES::disabledOn, QLatin1String("button-disabled"));
            }
        }

        if (propertyEmpty(button, TOKEN_PROPERTIES::disabledOff))
        {
            button->setProperty(TOKEN_PROPERTIES::disabledOff, QLatin1String("button-disabled"));
        }
    }
}

bool ButtonTokensByType::propertyEmpty(QAbstractButton* button, const char* propertyName)
{
    auto value = button->property(propertyName);
    return !value.isValid() || value.toString().isEmpty();
}
