#include "MegaItemDelegates.h"
#include "MegaApplication.h"
#include "MegaItemModel.h"

#include <QBitmap>
#include <QPainter>
#include <QToolTip>
#include <QFontMetrics>


IconDelegate::IconDelegate(QObject* parent) :
    QStyledItemDelegate(parent)
{
}

IconDelegate::~IconDelegate()
{
}


void IconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    opt.decorationAlignment = index.data(Qt::TextAlignmentRole).value<Qt::Alignment>();
    opt.decorationPosition = QStyleOptionViewItem::Top;
    opt.decorationSize = QSize(17, 17);
    QStyledItemDelegate::paint(painter, opt, index);
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
    if(indentValue < 0)
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
        if(index.column() == MegaItemModel::NODE)
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
