#ifndef ICONLABEL_H
#define ICONLABEL_H

#include "TokenizableItems.h"

#include <QToolButton>

#include <memory>

class TokenizableItem;

class IconLabel: public QToolButton, public TokenizableItem
{
    Q_OBJECT
    Q_PROPERTY(
        QString normal_off READ getNormalOffToken WRITE setNormalOffToken NOTIFY normalOffChanged)
    Q_PROPERTY(
        QString normal_on READ getNormalOnToken WRITE setNormalOnToken NOTIFY normalOnChanged)
    Q_PROPERTY(QString disabled_off READ getNormalOffToken WRITE setNormalOffToken NOTIFY
                   disabledOffChanged)
    Q_PROPERTY(
        QString disabled_on READ getNormalOffToken WRITE setNormalOffToken NOTIFY disabledOnChanged)

    void setNormalOffToken(const QString& normalOff) override
    {
        if (normalOff == getNormalOffToken())
        {
            return;
        }
        TokenizableItem::setNormalOffToken(normalOff);
        normalOffChanged(normalOff);
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

signals:
    void normalOffChanged(QString normalOff);
    void normalOnChanged(QString normalOn);
    void disabledOffChanged(QString disabledOff);
    void disabledOnChanged(QString disabledOn);

public:
    IconLabel(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // ICONLABEL_H
