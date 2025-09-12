#include "ButtonTokensByType.h"

#include "TokenPropertyNames.h"

#include <QVariant>

void ButtonTokensByType::setDefaultTokens(QAbstractButton* button)
{
    auto buttonType = button->property("type").toString();

    if (buttonType == QLatin1String("primary"))
    {
        if (propertyEmpty(button, normalOff))
        {
            button->setProperty(normalOff, QLatin1String("text-inverse-accent"));
        }
    }
    else if (buttonType == QLatin1String("secondary"))
    {
        if (propertyEmpty(button, normalOff))
        {
            button->setProperty(normalOff, QLatin1String("icon-secondary"));
        }
    }
    else if (buttonType == QLatin1String("outline") || buttonType == QLatin1String("ghost"))
    {
        if (propertyEmpty(button, normalOff))
        {
            button->setProperty(normalOff, QLatin1String("button-outline"));
        }

        if (propertyEmpty(button, hoverOff))
        {
            button->setProperty(hoverOff, QLatin1String("button-outline-hover"));
        }

        if (propertyEmpty(button, pressedOff))
        {
            button->setProperty(pressedOff, QLatin1String("button-outline-pressed"));
        }
    }
    else if (buttonType == QLatin1String("link"))
    {
        if (propertyEmpty(button, normalOff))
        {
            button->setProperty(normalOff, QLatin1String("link-primary"));
        }
    }
}

bool ButtonTokensByType::propertyEmpty(QAbstractButton* button, const char* propertyName)
{
    return !button->property(propertyName).isValid();
}
