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
    // Base token properties (inherited from TokenizableItem)
    Q_PROPERTY(
        QString normal_off READ getNormalOffToken WRITE setNormalOffToken NOTIFY normalOffChanged)
    Q_PROPERTY(
        QString normal_on READ getNormalOnToken WRITE setNormalOnToken NOTIFY normalOnChanged)
    Q_PROPERTY(QString disabled_off READ getDisabledOffToken WRITE setDisabledOffToken NOTIFY
                   disabledOffChanged)
    Q_PROPERTY(QString disabled_on READ getDisabledOnToken WRITE setDisabledOnToken NOTIFY
                   disabledOnChanged)

    // Button-specific properties
    Q_PROPERTY(QString pressed_off READ getPressedOffToken WRITE setPressedOffToken NOTIFY
                   pressedOffChanged)
    Q_PROPERTY(
        QString pressed_on READ getPressedOnToken WRITE setPressedOnToken NOTIFY pressedOnChanged)
    Q_PROPERTY(
        QString hover_off READ getHoverOffToken WRITE setHoverOffToken NOTIFY hoverOffChanged)
    Q_PROPERTY(QString hover_on READ getHoverOnToken WRITE setHoverOnToken NOTIFY hoverOnChanged)

    // Button-specific getters (delegate to ButtonTokens)
    QString getPressedOffToken() const
    {
        return buttonTokens.getPressedOffToken();
    }

    QString getPressedOnToken() const
    {
        return buttonTokens.getPressedOnToken();
    }

    QString getHoverOffToken() const
    {
        return buttonTokens.getHoverOffToken();
    }

    QString getHoverOnToken() const
    {
        return buttonTokens.getHoverOnToken();
    }

    // Base token setters
    void setNormalOffToken(const QString& normalOff) override
    {
        if (normalOff == getNormalOffToken())
        {
            return;
        }
        TokenizableItem::setNormalOffToken(normalOff);
        emit normalOffChanged(normalOff);
    }

    void setNormalOnToken(const QString& normalOn) override
    {
        if (normalOn == getNormalOnToken())
        {
            return;
        }
        TokenizableItem::setNormalOnToken(normalOn);
        emit normalOnChanged(normalOn);
    }

    void setDisabledOffToken(const QString& disabledOff) override
    {
        if (disabledOff == getDisabledOffToken())
        {
            return;
        }
        TokenizableItem::setDisabledOffToken(disabledOff);
        emit disabledOffChanged(disabledOff);
    }

    void setDisabledOnToken(const QString& disabledOn) override
    {
        if (disabledOn == getDisabledOnToken())
        {
            return;
        }
        TokenizableItem::setDisabledOnToken(disabledOn);
        emit disabledOnChanged(disabledOn);
    }

    // Button-specific setters
    void setPressedOffToken(const QString& pressedOff)
    {
        if (pressedOff == getPressedOffToken())
        {
            return;
        }
        buttonTokens.setPressedOffToken(pressedOff);
        emit pressedOffChanged(pressedOff);
    }

    void setPressedOnToken(const QString& pressedOn)
    {
        if (pressedOn == getPressedOnToken())
        {
            return;
        }
        buttonTokens.setPressedOnToken(pressedOn);
        emit pressedOnChanged(pressedOn);
    }

    void setHoverOffToken(const QString& hoverOff)
    {
        if (hoverOff == getHoverOffToken())
        {
            return;
        }
        buttonTokens.setHoverOffToken(hoverOff);
        emit hoverOffChanged(hoverOff);
    }

    void setHoverOnToken(const QString& hoverOn)
    {
        if (hoverOn == getHoverOnToken())
        {
            return;
        }
        buttonTokens.setHoverOnToken(hoverOn);
        emit hoverOnChanged(hoverOn);
    }

signals:
    // Base token signals
    void normalOffChanged(const QString& normalOff);
    void normalOnChanged(const QString& normalOn);
    void disabledOffChanged(const QString& disabledOff);
    void disabledOnChanged(const QString& disabledOn);

    // Button-specific signals
    void pressedOffChanged(const QString& pressedOff);
    void pressedOnChanged(const QString& pressedOn);
    void hoverOffChanged(const QString& hoverOff);
    void hoverOnChanged(const QString& hoverOn);

public:
    TokenizableButton(QWidget* parent = nullptr);

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
