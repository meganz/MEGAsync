#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"

#include <QBitmap>
#include <QPainter>
#include <QToolTip>
#include <QFontMetrics>
#include <QAbstractItemView>
#include <QApplication>


const int IconDelegate::ICON_HEIGHT = 18;

IconDelegate::IconDelegate(QObject* parent) :
    QStyledItemDelegate(parent)
{
}

IconDelegate::~IconDelegate()
{
}


void IconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItem opt(option);
    opt.decorationAlignment = index.data(Qt::TextAlignmentRole).value<Qt::Alignment>();

    QPixmap pixmap;
    if(index.data(Qt::DecorationRole).canConvert<QIcon>())
    {
        auto icon = index.data(Qt::DecorationRole).value<QIcon>();

        QIcon::Mode iconMode = QIcon::Normal;
        if(option.state.testFlag(QStyle::State_Selected))
        {
            iconMode = QIcon::Selected;
        }
        pixmap = icon.pixmap(ICON_HEIGHT, iconMode);
    }
    else
    {
        pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
    }

    const int x = option.rect.center().x() - ICON_HEIGHT / 2;
    const int y = option.rect.center().y() - ICON_HEIGHT / 2;

    painter->drawPixmap(QRect(x, y + 1, ICON_HEIGHT, ICON_HEIGHT), pixmap);
}

void IconDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->icon = QIcon();
}


const int NodeRowDelegate::MARGIN = 7;
const int NodeRowDelegate::ICON_MARGIN = 37;
const int NodeRowDelegate::DIFF_WITH_STD_ICON = 5;

NodeRowDelegate::NodeRowDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void NodeRowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    int indentValue = index.data(toInt(NodeRowDelegateRoles::INDENT_ROLE)).toInt();
    opt.rect.adjust(indentValue, 0, 0, 0);
    QVariant small_icon = index.data(toInt(NodeRowDelegateRoles::SMALL_ICON_ROLE));
    if(small_icon.isValid() && small_icon.toBool())
        opt.decorationSize = QSize(opt.decorationSize.width() - DIFF_WITH_STD_ICON, opt.decorationSize.height() - DIFF_WITH_STD_ICON);

    QStyledItemDelegate::paint(painter, opt, index);
}

bool NodeRowDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if ( !event || !view )
        return false;

    if (event->type() == QEvent::ToolTip)
    {
        QRect rect = view->visualRect(index);
        QString tooltipText = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm = option.fontMetrics;

        int margin = MARGIN;
        if(index.column() == NodeSelectorModel::NODE)
        {
            margin = ICON_MARGIN;
        }
        if (rect.width() < (fm.width(tooltipText) + margin))
        {
                QToolTip::showText(event->globalPos(), tooltipText);
                return true;
        }
        if ( !QStyledItemDelegate::helpEvent( event, view, option, index ) )
            QToolTip::hideText();
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index );
}

void NodeRowDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    QVariant enabled = index.data(toInt(NodeRowDelegateRoles::ENABLED_ROLE));
    if (enabled.isValid() && !enabled.toBool())
    {
        option->state &= ~QStyle::State_Enabled;
    }
}

DateColumnDelegate::DateColumnDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void DateColumnDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    QPalette::ColorGroup cg = QPalette::Normal;
    QVariant enabled = index.data(toInt(NodeRowDelegateRoles::ENABLED_ROLE));
    if (enabled.isValid() && !enabled.toBool())
    {
        cg = QPalette::Disabled;
    }
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }
    qDebug()<<option.palette.color(cg, QPalette::Text).name();
    QRect rect = option.rect;
    rect.adjust(10, 0, -5, 0);
    QString elideText = option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideMiddle, rect.width());
    painter->drawText(rect, Qt::AlignVCenter, elideText);
    painter->restore();
}

void DateColumnDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->text = QString::fromUtf8("");
}
