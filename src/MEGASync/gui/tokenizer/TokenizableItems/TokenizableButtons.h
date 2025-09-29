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
    TokenizableButton(QWidget* parent);

    void clear() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void init(QAbstractButton*) override;

private:
    ButtonTokens mButtonTokens;
};

class IconOnlyButton: public TokenizableButton
{
public:
    IconOnlyButton(QWidget* parent);
};

#endif // TOKENIZABLEBUTTONS_H
