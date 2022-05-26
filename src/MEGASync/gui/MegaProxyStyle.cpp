#include "MegaProxyStyle.h"
#include "gui/MegaTransferView.h"

#include <EventHelper.h>
#include <QStyleOption>
#include <QHeaderView>
#include <QSpinBox>
#include <QComboBox>

const int TOOLTIP_DELAY = 250;

void MegaProxyStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, true);
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void MegaProxyStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, true);
    switch(element)
    {
    case CE_HeaderLabel:
    {
        auto headerView = qobject_cast<const QHeaderView*>(widget);
        if(!headerView)
        {
            break;
        }

        if(!headerView->property("HeaderIconCenter").isValid()
                 || !headerView->property("HeaderIconCenter").toBool())
        {
            break;
        }

        if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option))
        {
            if(!header->icon.isNull() && header->text.isEmpty())
            {
                QRect rect = header->rect;
                if (!header->icon.isNull()) {
                    int size = qRound(headerView->height() * 0.8);
                    QPixmap pixmap
                        = header->icon.pixmap(QSize(size, size), (header->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled);

                    QRect aligned = alignedRect(header->direction, QFlag(Qt::AlignCenter), pixmap.size() / pixmap.devicePixelRatio(), rect);
                    QRect inter = aligned.intersected(rect);
                    painter->drawPixmap(inter.x(), inter.y(), pixmap,
                                  inter.x() - aligned.x(), inter.y() - aligned.y(),
                                  qRound(aligned.width() * pixmap.devicePixelRatio() + 0.5),
                                  qRound(pixmap.height() * pixmap.devicePixelRatio() + 0.5));
                    return;
            }
        }
        }
    }
    default:
        break;
    }

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
        QStyleOption modOption(*option);
        QColor c("#FF654F");

        QPen linepen(c);
        linepen.setCapStyle(Qt::RoundCap);
        linepen.setWidth(8);
        painter->setPen(linepen);
        painter->drawPoint(modOption.rect.topLeft() + QPoint(25, 0));
        painter->drawPoint(modOption.rect.topRight() - QPoint(25, 0));

        QPen whitepen(Qt::white);
        whitepen.setWidth(4);
        whitepen.setCapStyle(Qt::RoundCap);
        painter->setPen(whitepen);
        painter->drawPoint(modOption.rect.topLeft() + QPoint(25, 0));
        painter->drawPoint(modOption.rect.topRight() - QPoint(25, 0));

        modOption.rect.setLeft(30);
        modOption.rect.setRight(modOption.rect.width());
        linepen.setWidth(2);
        painter->setPen(linepen);

        QProxyStyle::drawPrimitive(element, &modOption, painter, widget);

        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int MegaProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption * option, const QWidget * widget) const
{
    if(metric == QStyle::PM_SmallIconSize)
    {
        return 24;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

int MegaProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_ToolTip_WakeUpDelay)
    {
        return TOOLTIP_DELAY;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
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

void MegaProxyStyle::polish(QPalette &pal)
{
    QProxyStyle::polish(pal);
}

void MegaProxyStyle::polish(QApplication *app)
{
    QProxyStyle::polish(app);
}

void MegaProxyStyle::unpolish(QWidget *widget)
{
    QProxyStyle::unpolish(widget);
}

void MegaProxyStyle::unpolish(QApplication *app)
{
    QProxyStyle::unpolish(app);
}

bool MegaProxyStyle::event(QEvent *e)
{
    return QProxyStyle::event(e);
}
