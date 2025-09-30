#ifndef TOKENIZABLEBUTTONS_H
#define TOKENIZABLEBUTTONS_H

#include "IconProperty.h"
#include "TokenizableItems.h"

#include <QPushButton>

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

class TokenizableButton: public QPushButton, public TokenizableItem
{
    Q_OBJECT

public:
    TokenizableButton(QWidget* parent = nullptr);
    
    void forcePress();
    void forceMouseOver();
    void resetForcedState();

    DEFINE_ICON_PROPERTY()

protected:
    void paintEvent(QPaintEvent* event) override;
    void init(QAbstractButton*) override;

private:
    ButtonTokens mButtonTokens;
    QStyle::State mForcedState;
};

class IconOnlyButton: public TokenizableButton
{
public:
    IconOnlyButton(QWidget* parent);
};

#endif // TOKENIZABLEBUTTONS_H
