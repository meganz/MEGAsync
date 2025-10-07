#include "TokenizableButtons.h"

#include "ButtonTokensByType.h"
#include "TokenizableItems/TokenizableItems.h"

void TokenizableAbstractButton::applyPixmapsByState(QAbstractButton* button, QStyleOption option)
{
    if (mForcedState != QStyle::State_None)
    {
        option.state |= mForcedState;
    }

    if (stateHasChanged(option))
    {
        QIcon::State state =
            option.state.testFlag(QStyle::State_On) ? QIcon::State::On : QIcon::State::Off;

        QIcon::Mode mode = button->hasFocus() ? QIcon::Active : QIcon::Normal;

        QString token;

        // Sunken goes before MouseOver because it you are pressing, you are also hovering the
        // button
        if (option.state.testFlag(QStyle::State_Sunken))
        {
            token = state == QIcon::State::Off ? mButtonTokens.getPressedOffToken() :
                                                 mButtonTokens.getPressedOnToken();
            applyPixmap(button, token, mode, state);
        }
        else if (option.state.testFlag(QStyle::State_MouseOver))
        {
            token = state == QIcon::State::Off ? mButtonTokens.getHoverOffToken() :
                                                 mButtonTokens.getHoverOnToken();
            applyPixmap(button, token, mode, state);
        }

        // If we don´t have any token for the rest of states, set the new token for the default
        // states
        if (token.isEmpty())
        {
            applyDefaultPixmap(button);
        }
    }
}

void TokenizableAbstractButton::init(QAbstractButton* button)
{
    mButtonTokens.fillTokens(button);

    if (!isInitialized())
    {
        // If no dynamic properties were added, add the default button properties
        ButtonTokensByType::setDefaultTokens(button);
    }

    if (mButtonTokens.anyTokenHasChanged())
    {
        forceUpdate();
    }

    TokenizableItem::init(button);
}

void TokenizableAbstractButton::forcePress()
{
    mForcedState = QStyle::State_Sunken;
}

void TokenizableAbstractButton::forceMouseOver()
{
    mForcedState = QStyle::State_MouseOver;
}

void TokenizableAbstractButton::resetForcedState()
{
    mForcedState = QStyle::State_None;
}

/********************/
TokenizableButton::TokenizableButton(QWidget* parent):
    QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void TokenizableButton::paintEvent(QPaintEvent* event)
{
    QStyleOptionButton option;
    initStyleOption(&option);

    // The first time the button is painted
    init(this);

    applyPixmapsByState(this, option);

    QPushButton::paintEvent(event);
}

/*************** TOOL BUTTON ***************/
TokenizableToolButton::TokenizableToolButton(QWidget* parent):
    QToolButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setToolButtonStyle(toolButtonStyle());
}

void TokenizableToolButton::setToolButtonStyle(Qt::ToolButtonStyle style)
{
    QToolButton::setToolButtonStyle(style);
    QToolButton::setProperty("class",
                             style == Qt::ToolButtonIconOnly ? QLatin1String("icon-button") :
                                                               QVariant());
}

void TokenizableToolButton::paintEvent(QPaintEvent* event)
{
    QStyleOptionToolButton option;
    initStyleOption(&option);

    // The first time the button is painted
    init(this);

    applyPixmapsByState(this, option);

    QToolButton::paintEvent(event);
}
