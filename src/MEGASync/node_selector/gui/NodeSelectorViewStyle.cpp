#include "NodeSelectorViewStyle.h"

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
            QStyleOptionViewItem iconOnly(*viewItem);
            iconOnly.text.clear();
            iconOnly.features &= ~QStyleOptionViewItem::HasDisplay;
            MegaProxyStyle::drawControl(element, &iconOnly, painter, widget);

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
