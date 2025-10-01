#include "IconLabel.h"

#include "TokenizableItems/TokenizableItems.h"

IconLabel::IconLabel(QWidget* parent):
    QToolButton(parent)
{
    QToolButton::setProperty("class", QLatin1String("icon-label"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setText(QString());
}

void IconLabel::paintEvent(QPaintEvent* event)
{
    QStyleOptionToolButton option;
    initStyleOption(&option);

    // The first time the button is painted
    init(this);

    if (stateHasChanged(option) || mBaseTokens.anyTokenHasChanged())
    {
        applyDefaultPixmap(this);
    }

    QToolButton::paintEvent(event);
}
