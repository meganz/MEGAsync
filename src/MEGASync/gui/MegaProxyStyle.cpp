#include "MegaProxyStyle.h"
#include "gui/MegaTransferView.h"
#include <QStyleOption>
#include <QSpinBox>
#include <QComboBox>
#include <EventHelper.h>

void MegaProxyStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, true);
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void MegaProxyStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, true);
    QProxyStyle::drawControl(element, option, painter, widget);
}

void MegaProxyStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, true);
    QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

void MegaProxyStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, true);
    QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void MegaProxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

    if (element == QStyle::PE_IndicatorItemViewItemDrop && !option->rect.isNull())
    {
        QStyleOption opt(*option);
        const MegaTransferView *transferView = dynamic_cast<const MegaTransferView *>(widget);
        if (transferView)
        {
            int type = transferView->getType();
            if (type == mega::MegaTransfer::TYPE_DOWNLOAD || type == mega::MegaTransfer::TYPE_UPLOAD)
            {
                QColor c(type == mega::MegaTransfer::TYPE_DOWNLOAD ? "#31B500" : "#2BA6DE");

                QPen linepen(c);
                linepen.setCapStyle(Qt::RoundCap);
                linepen.setWidth(8);
                painter->setPen(linepen);
                painter->drawPoint(opt.rect.topLeft() + QPoint(30, 0));
                painter->drawPoint(opt.rect.topRight() - QPoint(30, 0));

                QPen whitepen(Qt::white);
                whitepen.setWidth(4);
                whitepen.setCapStyle(Qt::RoundCap);
                painter->setPen(whitepen);
                painter->drawPoint(opt.rect.topLeft() + QPoint(30, 0));
                painter->drawPoint(opt.rect.topRight() - QPoint(30, 0));

                opt.rect.setLeft(35);
                opt.rect.setRight(widget->width() - 35);
                linepen.setWidth(2);
                painter->setPen(linepen);
            }
        }

        QProxyStyle::drawPrimitive(element, &opt, painter, widget);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int MegaProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption * option, const QWidget * widget) const
{
    if (metric == QStyle::PM_SmallIconSize)
        return 24;
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QIcon MegaProxyStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    switch (standardIcon)
    {
        case SP_MessageBoxInformation:
            return QIcon(QString::fromAscii("://images/icon_info.png"));
        case SP_MessageBoxQuestion:
            return QIcon(QString::fromAscii("://images/icon_question.png"));
        case SP_MessageBoxCritical:
            return QIcon(QString::fromAscii("://images/icon_error.png"));
        case SP_MessageBoxWarning:
            return QIcon(QString::fromAscii("://images/icon_warning.png"));
        default:
            break;
    }
    return QProxyStyle::standardIcon(standardIcon, option, widget);
}

void MegaProxyStyle::polish(QWidget *widget)
{
    if(auto spinBox = qobject_cast<QSpinBox*>(widget))
    {
        EventManager::addEvent(spinBox, QEvent::Wheel, EventHelper::BLOCK);
    }
    else if(auto comboBox = qobject_cast<QComboBox*>(widget))
    {
        EventManager::addEvent(comboBox, QEvent::Wheel, EventHelper::BLOCK);
    }
    QProxyStyle::polish(widget);
}
