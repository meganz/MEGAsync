#include "TokenizableButtons.h"

#include "ButtonTokensByType.h"
#include "TokenizableItems/TokenizableItems.h"

/********************/
TokenizableButton::TokenizableButton(QWidget* parent):
    QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void TokenizableButton::forceUpdate()
{
    TokenizableItem::forceUpdate();
}

void TokenizableButton::paintEvent(QPaintEvent* event)
{
    QStyleOptionButton option;
    initStyleOption(&option);
    if (mForcedState != QStyle::State_None)
    {
        option.state |= mForcedState;
    }

    // The first time the button is painted
    init(this);

    if (stateHasChanged(option))
    {
        QIcon::State state =
            option.state.testFlag(QStyle::State_On) ? QIcon::State::On : QIcon::State::Off;

        QIcon::Mode mode = hasFocus() ? QIcon::Active : QIcon::Normal;

        // Sunken goes before MouseOver because it you are pressing, you are also hovering the
        // button
        if (option.state.testFlag(QStyle::State_Sunken))
        {
            auto token = state == QIcon::State::Off ? mButtonTokens.getPressedOffToken() :
                                                      mButtonTokens.getPressedOnToken();
            applyPixmap(this, token, mode, state);
        }
        else if (option.state.testFlag(QStyle::State_MouseOver))
        {
            auto token = state == QIcon::State::Off ? mButtonTokens.getHoverOffToken() :
                                                      mButtonTokens.getHoverOnToken();
            applyPixmap(this, token, mode, state);
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
    mButtonTokens.fillTokens(this);

    if (!isInitialized())
    {
        // If no dynamic properties were added, add the default button properties
        ButtonTokensByType::setDefaultTokens(this);
    }

    if (mButtonTokens.anyTokenHasChanged())
    {
        forceUpdate();
    }

    TokenizableItem::init(this);
}

void TokenizableButton::forcePress()
{
    mForcedState = QStyle::State_Sunken;
}

void TokenizableButton::forceMouseOver()
{
    mForcedState = QStyle::State_MouseOver;
}

void TokenizableButton::resetForcedState()
{
    mForcedState = QStyle::State_None;
}

void TokenizableButton::setIcon(const QIcon& icon)
{
    QPushButton::setIcon(icon);
    forceUpdate();
}

/*************** ONLY_ICON_BUTTON ***************/
IconOnlyButton::IconOnlyButton(QWidget* parent):
    TokenizableButton(parent)
{
    QPushButton::setProperty("class", QLatin1String("icon-button"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setText(QString());
}
