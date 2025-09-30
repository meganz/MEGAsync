#ifndef ICONLABEL_H
#define ICONLABEL_H

#include "TokenizableItems.h"

#include <QToolButton>

#include <memory>

class TokenizableItem;

class IconLabel: public QToolButton, public TokenizableItem
{
    Q_OBJECT

public:
    Q_PROPERTY(QIcon icon WRITE setIcon)

    IconLabel(QWidget* parent);

    void setIcon(const QIcon& icon);

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // ICONLABEL_H
