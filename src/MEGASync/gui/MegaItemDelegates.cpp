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
    painter->save();

    QStyledItemDelegate::paint(painter, option, index);

    QRect rect = option.rect;
    int height = qRound(option.rect.height() * 0.8);
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QPixmap pm;
    if(option.state & QStyle::State_Selected)
    {
        pm = icon.pixmap(height, height, QIcon::Selected);
    }
    else
    {
        pm = icon.pixmap(height, height, QIcon::Normal);
    }

    Qt::Alignment alignment = index.data(Qt::TextAlignmentRole).value<Qt::Alignment>();

    rect.setSize(QSize(height, height));
    if(alignment & Qt::AlignHCenter)
    {
        rect.moveCenter(option.rect.center());
    }
    else if(alignment & Qt::AlignLeft)
    {
        QPoint p = option.rect.center();
        p.setX(option.rect.left() + height);
        rect.moveCenter(p);
    }
    else if(alignment & Qt::AlignRight)
    {
        QPoint p = option.rect.center();
        p.setX(option.rect.right() - height);
        rect.moveCenter(p);
    }
    painter->drawPixmap(rect, pm);
    painter->restore();
}

void IconDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->icon = QIcon();
}



const int NodeRowDelegate::MARGIN = 7;
const int NodeRowDelegate::ICON_MARGIN = 37;

NodeRowDelegate::NodeRowDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void NodeRowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    int indentValue = index.data(toInt(NodeRowDelegateRoles::INDENT_ROLE)).toInt();
    opt.rect.adjust(indentValue, 0, 0, 0);
    QStyledItemDelegate::paint(painter, option, index);
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
