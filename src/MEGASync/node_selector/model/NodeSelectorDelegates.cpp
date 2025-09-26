#include "NodeSelectorDelegates.h"

#include "MegaDelegateHoverManager.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorTreeView.h"
#include "TokenParserWidgetManager.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QBitmap>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

NodeSelectorDelegate::NodeSelectorDelegate(QObject* parent):
    QStyledItemDelegate(parent)
{}

void NodeSelectorDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    QStyleOptionViewItem auxOpt(option);

    if (!index.data(toInt(NodeSelectorModelRoles::EXTRA_ROW_ROLE)).toBool())
    {
        auto pen(painter->pen());
        pen.setWidth(1);

        // Text color
        if (!index.flags().testFlag(Qt::ItemIsEnabled))
        {
            auxOpt.palette.setBrush(
                QPalette::ColorRole::Text,
                TokenParserWidgetManager::instance()->getColor(QLatin1String("text-disabled")));
        }
        else
        {
            auxOpt.palette.setBrush(
                QPalette::ColorRole::Text,
                TokenParserWidgetManager::instance()->getColor(QLatin1String("text-primary")));
            auxOpt.palette.setBrush(
                QPalette::ColorRole::HighlightedText,
                TokenParserWidgetManager::instance()->getColor(QLatin1String("text-primary")));
        }

        // Separator
        {
            painter->save();
            pen.setColor(
                TokenParserWidgetManager::instance()->getColor(QLatin1String("border-subtle")));
            painter->setPen(pen);

            int y = option.rect.bottomLeft().y();
            int leftX = index.column() == 0 ? 0 : option.rect.x();
            int rightX = option.rect.x();
            rightX += index.column() == index.model()->columnCount() - 1 ?
                          (option.rect.width() - 10) :
                          option.rect.width();

            auto line = QLine(QPoint(leftX, y), QPoint(rightX, y));

            painter->drawLine(line);
            painter->restore();
        }

        // Adjust the content to align it with the header
        auxOpt.rect.adjust(7, 0, -5, 0);
    }

    auxOpt.state.setFlag(QStyle::State_MouseOver, false);
    auxOpt.state.setFlag(QStyle::State_Selected, false);

    QStyledItemDelegate::paint(painter, auxOpt, index);
}

bool NodeSelectorDelegate::isHoverStateSet(const QModelIndex& index)
{
    if (!mLastHoverRow.isValid())
    {
        return false;
    }

    if (mLastHoverRow.data(toInt(NodeSelectorModelRoles::EXTRA_ROW_ROLE)).toBool())
    {
        return false;
    }

    return (mLastHoverRow.parent() == index.parent() && mLastHoverRow.row() == index.row());
}

bool NodeSelectorDelegate::event(QEvent* event)
{
    if (auto hoverEvent = dynamic_cast<MegaDelegateHoverEvent*>(event))
    {
        switch (static_cast<int>(hoverEvent->type()))
        {
            case MegaDelegateHoverEvent::Enter:
            {
                mLastHoverRow = hoverEvent->index();
                break;
            }
            case MegaDelegateHoverEvent::Leave:
            {
                mLastHoverRow = QModelIndex();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return QStyledItemDelegate::event(event);
}

const int NodeRowDelegate::MARGIN = 7;
const int NodeRowDelegate::ICON_MARGIN = 37;
const int NodeRowDelegate::DIFF_WITH_STD_ICON = 5;

NodeRowDelegate::NodeRowDelegate(QObject* parent):
    NodeSelectorDelegate(parent)
{}

void NodeRowDelegate::paint(QPainter* painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);

    opt.displayAlignment = Qt::AlignVCenter | Qt::AlignLeft;
    opt.decorationAlignment = Qt::AlignVCenter | Qt::AlignLeft;
    opt.decorationSize = index.data(toInt(NodeSelectorModelRoles::ICON_SIZE_ROLE)).toSize();

    NodeSelectorDelegate::paint(painter, opt, index);
}

QPixmap NodeRowDelegate::paintForDrag(const QModelIndex& index, QAbstractItemView* view) const
{
    QRect rect = view->visualRect(index);
    // Limit the width to 400px max
    rect.setWidth(std::min(rect.width(), 400));
    QPixmap pixmap(rect.size());
    pixmap.fill(Qt::transparent);

    QStyleOptionViewItem option;
    option.initFrom(view);
    option.rect = QRect(0, 0, rect.width(), rect.height());

    QPainter painter(&pixmap);

    QPainterPath path;
    auto backgroundRect(option.rect);
    backgroundRect.setRight(option.rect.right() - 10);
    backgroundRect.setTop(option.rect.top() + 3);
    backgroundRect.setBottom(option.rect.bottom() - 5);
    path.addRoundedRect(backgroundRect, 4, 4);
    painter.fillPath(path,
                     TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-2")));

    paint(&painter, option, index);
    painter.end();

    return pixmap;
}

bool NodeRowDelegate::helpEvent(QHelpEvent* event,
                                QAbstractItemView* view,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index)
{
    if (!event || !view || !index.isValid())
    {
        return false;
    }

    if (event->type() == QEvent::ToolTip)
    {
        QRect rect = view->visualRect(index);
        QString tooltipText = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm = option.fontMetrics;

        int margin = MARGIN;
        if (index.column() == NodeSelectorModel::NODE)
        {
            margin = ICON_MARGIN;
        }
        if (rect.width() < (fm.horizontalAdvance(tooltipText) + margin))
        {
            QToolTip::showText(event->globalPos(), tooltipText.toHtmlEscaped());
            return true;
        }
        if (!QStyledItemDelegate::helpEvent(event, view, option, index))
        {
            QToolTip::hideText();
        }
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize NodeRowDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto size = NodeSelectorDelegate::sizeHint(option, index);
    size.setHeight(49); // 48 + 1 border pixel
    return size;
}

void NodeRowDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if (!index.flags().testFlag(Qt::ItemIsEnabled))
    {
        option->state &= ~QStyle::State_Enabled;
    }
}
