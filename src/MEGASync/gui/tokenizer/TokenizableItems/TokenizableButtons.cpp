#include "TokenizableButtons.h"

#include "ButtonTokensByType.h"
#include "TokenizableItems/TokenizableItems.h"

/********************/
TokenizableButton::TokenizableButton(QWidget* parent):
    QPushButton(parent)
{}

void TokenizableButton::paintEvent(QPaintEvent* event)
{
    QStyleOptionButton option;
    initStyleOption(&option);

    // The first time the button is painted
    init(this);

    if (stateHasChanged(option))
    {
        QIcon::State state =
            option.state.testFlag(QStyle::State_On) ? QIcon::State::On : QIcon::State::Off;

        // Sunken goes before MouseOver because it you are pressing, you are also hovering the
        // button
        if (option.state.testFlag(QStyle::State_Sunken))
        {
            auto token = state == QIcon::State::Off ? buttonTokens.getPressedOffToken() :
                                                      buttonTokens.getPressedOnToken();
            applyPixmap(this, token, QIcon::Normal, state);
        }
        else if (option.state.testFlag(QStyle::State_MouseOver))
        {
            auto token = state == QIcon::State::Off ? buttonTokens.getHoverOffToken() :
                                                      buttonTokens.getHoverOnToken();
            applyPixmap(this, token, QIcon::Normal, state);
        }
        else
        {
            applyDefaultPixmap(this);
        }
    }

    QPushButton::paintEvent(event);
}

void TokenizableButton::init(QAbstractButton*)
{
    if (!isInitialized())
    {
        // If no dynamic properties were added, add the default button properties
        ButtonTokensByType::setDefaultTokens(this);

        buttonTokens.fillTokens(this);

        TokenizableItem::init(this);
    }
}

/*************** ONLY_ICON_BUTTON ***************/
IconOnlyButton::IconOnlyButton(QWidget* parent):
    TokenizableButton(parent)
{
    QPushButton::setProperty("class", QLatin1String("icon-button"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setText(QString());
}
