#ifndef ICONLABEL_H
#define ICONLABEL_H

#include "IconProperty.h"
#include "TokenizableItems.h"

#include <QToolButton>

#include <memory>

class TokenizableItem;

class IconLabel: public QToolButton, public TokenizableItem
{
    Q_OBJECT

public:
    IconLabel(QWidget* parent);
    DEFINE_ICON_PROPERTY()

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // ICONLABEL_H
