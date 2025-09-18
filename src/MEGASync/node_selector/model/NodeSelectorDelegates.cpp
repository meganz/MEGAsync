#include "NodeSelectorDelegates.h"

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
    QStyledItemDelegate(parent),
    mMainDevice(nullptr)
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
            int leftX = index.column() == 0 ? 0 : leftX = option.rect.x();
            int rightX = option.rect.x();
            rightX += index.column() == index.model()->columnCount() - 1 ?
                          (option.rect.width() - 10) :
                          option.rect.width();

            auto line = QLine(QPoint(leftX, y), QPoint(rightX, y));

            painter->drawLine(line);
            painter->restore();
        }
    }

    QStyledItemDelegate::paint(painter, auxOpt, index);
}

void NodeSelectorDelegate::setPaintDevice(QPainter* painter, const QModelIndex& index) const
{
    if ((index.row() != 0 && index.column() != 0) || index.parent().isValid())
    {
        return;
    }

#ifndef Q_OS_MACOS
    // On Linux/Windows the main device is not permanent, it may change
    auto view = dynamic_cast<NodeSelectorTreeView*>(parent());
    if (mMainDevice != painter->device() && view->state() == NodeSelectorTreeView::NoState)
    {
        mMainDevice = painter->device();
    }
#else
    // First time the row is painted, we get the main paint device
    // in order to compare with other paint devices, as the use by the dragging action
    if (!mMainDevice)
    {
        mMainDevice = painter->device();
    }
#endif
}

bool NodeSelectorDelegate::isPaintingDrag(QPainter* painter) const
{
    auto view = dynamic_cast<NodeSelectorTreeView*>(parent());
    if (view)
    {
        return view->state() == NodeSelectorTreeView::DraggingState &&
               painter->device() != mMainDevice;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////7

const int IconDelegate::ICON_HEIGHT = 18;

IconDelegate::IconDelegate(QObject* parent):
    NodeSelectorDelegate(parent)
{}

IconDelegate::~IconDelegate() {}

void IconDelegate::paint(QPainter* painter,
                         const QStyleOptionViewItem& option,
                         const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);

    NodeSelectorDelegate::setPaintDevice(painter, index);

    if (!isPaintingDrag(painter))
    {
        NodeSelectorDelegate::paint(painter, option, index);
        opt.decorationAlignment = index.data(Qt::TextAlignmentRole).value<Qt::Alignment>();

        QIcon icon;
        QIcon::Mode iconMode = QIcon::Normal;
        if (index.data(Qt::DecorationRole).canConvert<QIcon>())
        {
            icon = index.data(Qt::DecorationRole).value<QIcon>();

            if (option.state.testFlag(QStyle::State_Selected))
            {
                iconMode = QIcon::Selected;
            }
        }
        else if (index.data(Qt::DecorationRole).canConvert<QPixmap>())
        {
            icon.addPixmap(index.data(Qt::DecorationRole).value<QPixmap>());
        }

        QRect iconRect(QPoint(option.rect.topLeft()), QSize(ICON_HEIGHT, ICON_HEIGHT));
        iconRect.moveCenter(option.rect.center());
        icon.paint(painter, iconRect, Qt::AlignVCenter | Qt::AlignHCenter, iconMode);
    }
}

void IconDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->icon = QIcon();
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

    NodeSelectorDelegate::setPaintDevice(painter, index);

    int indentValue = index.data(toInt(NodeRowDelegateRoles::INDENT_ROLE)).toInt();
    opt.rect.adjust(indentValue, 0, 0, 0);
    QVariant small_icon = index.data(toInt(NodeRowDelegateRoles::SMALL_ICON_ROLE));
    if (small_icon.isValid() && small_icon.toBool())
        opt.decorationSize = QSize(opt.decorationSize.width() - DIFF_WITH_STD_ICON,
                                   opt.decorationSize.height() - DIFF_WITH_STD_ICON);

    if (isPaintingDrag(painter))
    {
        QPainterPath selectedPath;
        selectedPath
            .addRoundedRect(opt.rect.x(), opt.rect.y(), opt.rect.width(), opt.rect.height(), 4, 4);
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(opt.palette.highlight());
        painter->drawPath(selectedPath);
        painter->restore();

        opt.state.setFlag(QStyle::State_Selected, false);
    }

    NodeSelectorDelegate::paint(painter, opt, index);
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

TextColumnDelegate::TextColumnDelegate(QObject* parent):
    NodeSelectorDelegate(parent)
{}

void TextColumnDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);

    NodeSelectorDelegate::setPaintDevice(painter, index);

    if (!isPaintingDrag(painter))
    {
        NodeSelectorDelegate::paint(painter, opt, index);
        painter->save();

        QRect rect = opt.rect;
        rect.adjust(10, 0, -5, 0);
        QString elideText = opt.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(),
                                                       Qt::ElideMiddle,
                                                       rect.width());
        painter->drawText(rect, Qt::AlignVCenter, elideText);
        painter->restore();
    }
}

bool TextColumnDelegate::helpEvent(QHelpEvent* event,
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
        rect.adjust(10, 0, -5, 0);
        QString tooltipText = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm = option.fontMetrics;

        if (rect.width() < (fm.horizontalAdvance(tooltipText)))
        {
            QToolTip::showText(event->globalPos(), tooltipText);
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

void TextColumnDelegate::initStyleOption(QStyleOptionViewItem* option,
                                         const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->text = QString::fromUtf8("");
}
