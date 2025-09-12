#ifndef TOKENIZABLEBUTTONS_H
#define TOKENIZABLEBUTTONS_H

#include "TokenizableItems.h"

#include <QPushButton>

class ButtonTokens
{
public:
    ButtonTokens() = default;

    void fillTokens(QAbstractButton* button)
    {
        mPressedOff = button->property(TOKEN_PROPERTIES::pressedOff).toString();
        mPressedOn = button->property(TOKEN_PROPERTIES::pressedOn).toString();
        mHoverOff = button->property(TOKEN_PROPERTIES::hoverOff).toString();
        mHoverOn = button->property(TOKEN_PROPERTIES::hoverOn).toString();
    }

    // hover_off
    QString getHoverOffToken() const
    {
        return mHoverOff;
    }

    // hover_on
    QString getHoverOnToken() const
    {
        return mHoverOn;
    }

    // pressed_off
    QString getPressedOffToken() const
    {
        return mPressedOff;
    }

    // pressed_on
    QString getPressedOnToken() const
    {
        return mPressedOn;
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
    TokenizableButton(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
    void init(QAbstractButton*) override;

private:
    ButtonTokens buttonTokens;
};

class IconOnlyButton: public TokenizableButton
{
public:
    IconOnlyButton(QWidget* parent);
};

#endif // TOKENIZABLEBUTTONS_H
