#ifndef TOKENIZABLEBUTTONS_H
#define TOKENIZABLEBUTTONS_H

#include "IconProperty.h"
#include "TokenizableItems.h"

#include <QPushButton>
#include <QToolButton>

namespace ButtonUtilities
{
inline void checkMinWidth(QAbstractButton* button,
                          Qt::ToolButtonStyle style = Qt::ToolButtonStyle::ToolButtonTextBesideIcon)
{
    static const char* dimensionProperty = "dimension";
    static const char* shortProperty = "short";
    static const char* currentNameProperty = "current-name";

    if (button->text() != button->property(currentNameProperty).toString())
    {
        button->setProperty(currentNameProperty, button->text());

        auto dimension = button->property(dimensionProperty).toString();
        if (!dimension.isEmpty())
        {
            auto fm = button->fontMetrics();
            auto contentsWidth = fm.boundingRect(button->text()).width();
            auto iconSize(button->icon().isNull() ? 0 : button->iconSize().width());

            if (style == Qt::ToolButtonStyle::ToolButtonTextBesideIcon)
            {
                contentsWidth += iconSize + 4;
            }
            else if (style == Qt::ToolButtonStyle::ToolButtonTextUnderIcon)
            {
                contentsWidth = std::max(contentsWidth, iconSize);
            }
            else if (style == Qt::ToolButtonStyle::ToolButtonIconOnly)
            {
                contentsWidth = iconSize;
            }

            QVariant currentValue = button->property(shortProperty);
            bool newValue(false);

            // Please check this values from WidgetsComponentsStyleSheetsSizes.css
            if (dimension == QLatin1String("small"))
            {
                newValue = contentsWidth < 44;
            }
            else if (dimension == QLatin1String("medium"))
            {
                newValue = contentsWidth < 48;
            }
            else if (dimension == QLatin1String("large"))
            {
                newValue = contentsWidth < 52;
            }

            if (!currentValue.isValid() || currentValue.toBool() != newValue)
            {
                button->setProperty(shortProperty, newValue);
                button->style()->polish(button);
            }
        }
    }
}
}

class ButtonTokens: public TokenUtilities
{
public:
    ButtonTokens() = default;

    void fillTokens(QAbstractButton* button)
    {
        fillToken(mPressedOff, TOKEN_PROPERTIES::pressedOff, button);
        fillToken(mPressedOn, TOKEN_PROPERTIES::pressedOn, button);
        fillToken(mHoverOff, TOKEN_PROPERTIES::hoverOff, button);
        fillToken(mHoverOn, TOKEN_PROPERTIES::hoverOn, button);
    }

    // hover_off
    QString getHoverOffToken() const
    {
        return mHoverOff;
    }

    void setHoverOff(const QString& newHoverOff)
    {
        mHoverOff = newHoverOff;
    }

    // hover_on
    QString getHoverOnToken() const
    {
        return mHoverOn;
    }

    void setHoverOn(const QString& newHoverOn)
    {
        mHoverOn = newHoverOn;
    }

    // pressed_off
    QString getPressedOffToken() const
    {
        return mPressedOff;
    }

    void setPressedOff(const QString& newPressedOff)
    {
        mHoverOff = newPressedOff;
    }

    // pressed_on
    QString getPressedOnToken() const
    {
        return mPressedOn;
    }

    void setPressedOn(const QString& newPressedOn)
    {
        mPressedOn = newPressedOn;
    }

private:
    QString mPressedOff;
    QString mPressedOn;
    QString mHoverOff;
    QString mHoverOn;
};

class TokenizableAbstractButton: public TokenizableItem
{
public:
    void forcePress();
    void forceMouseOver();
    void resetForcedState();

protected:
    TokenizableAbstractButton() = default;
    void applyPixmapsByState(QAbstractButton* button, QStyleOption option);
    void init(QAbstractButton* button) override;

private:
    ButtonTokens mButtonTokens;
    QStyle::State mForcedState;
};

class TokenizableButton: public QPushButton, public TokenizableAbstractButton
{
    Q_OBJECT

public:
    TokenizableButton(QWidget* parent = nullptr);

    DEFINE_ICON_PROPERTY()

protected:
    void leaveEvent(QEvent* event) override;

    void paintEvent(QPaintEvent* event) override;
};

class TokenizableToolButton: public QToolButton, public TokenizableAbstractButton
{
    Q_OBJECT

public:
    TokenizableToolButton(QWidget* parent = nullptr);

    void setToolButtonStyle(Qt::ToolButtonStyle style);

    DEFINE_ICON_PROPERTY()

protected:
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

#endif // TOKENIZABLEBUTTONS_H
