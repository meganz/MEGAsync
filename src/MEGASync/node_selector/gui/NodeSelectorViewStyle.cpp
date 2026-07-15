#include "NodeSelectorViewStyle.h"

#include <QIcon>
#include <QPainter>
#include <QStyleOptionViewItem>

QRect NodeSelectorViewStyle::subElementRect(SubElement element,
                                            const QStyleOption* option,
                                            const QWidget* widget) const
{
    QRect rect = MegaProxyStyle::subElementRect(element, option, widget);

    if (element == SE_ItemViewItemText)
    {
        if (const auto* viewItem = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            viewItem && !viewItem->icon.isNull())
        {
            // The icon is left-aligned within the item, so its right edge is the item left plus
            // the icon width. Anchor the text a fixed gap after it. This is honoured by code paths
            // that query the rect directly (e.g. the search delegate's manual text draw).
            const int iconRight = viewItem->rect.left() + viewItem->decorationSize.width();
            rect.setLeft(iconRight + ICON_TEXT_SPACING);
        }
    }

    return rect;
}

void NodeSelectorViewStyle::drawControl(ControlElement element,
                                        const QStyleOption* option,
                                        QPainter* painter,
                                        const QWidget* widget) const
{
    // Items are drawn through QStyleSheetStyle (the tree uses a stylesheet), which positions the
    // text via its own layout and ignores subElementRect(). To pin the icon-to-text gap we draw
    // the text ourselves: forward the item without its text (so only background + icon are drawn)
    // and then paint the text anchored a fixed gap after the icon.
    if (element == CE_ItemViewItem)
    {
        if (const auto* viewItem = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            viewItem && !viewItem->icon.isNull() && !viewItem->text.isEmpty())
        {
            QStyleOptionViewItem background(*viewItem);
            background.text.clear();
            background.features &= ~QStyleOptionViewItem::HasDisplay;
            background.icon = QIcon();
            background.features &= ~QStyleOptionViewItem::HasDecoration;
            MegaProxyStyle::drawControl(element, &background, painter, widget);

            // Paint the icon ourselves, left-aligned and vertically centred, so it is not clipped.
            const QSize decorationSize = viewItem->decorationSize;
            const QRect iconRect(viewItem->rect.left(),
                                 viewItem->rect.top() +
                                     (viewItem->rect.height() - decorationSize.height()) / 2,
                                 decorationSize.width(),
                                 decorationSize.height());
            viewItem->icon.paint(painter,
                                 iconRect,
                                 Qt::AlignVCenter | Qt::AlignLeft,
                                 QIcon::Normal,
                                 QIcon::On);

            const int iconRight = viewItem->rect.left() + viewItem->decorationSize.width();
            QRect textRect = viewItem->rect;
            textRect.setLeft(iconRight + ICON_TEXT_SPACING);

            const QFontMetrics metrics(viewItem->font);
            const QString elided =
                metrics.elidedText(viewItem->text, viewItem->textElideMode, textRect.width());

            painter->save();
            painter->setFont(viewItem->font);
            painter->setPen(viewItem->palette.color(QPalette::Text));
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);
            painter->restore();
            return;
        }
    }

    MegaProxyStyle::drawControl(element, option, painter, widget);
}
