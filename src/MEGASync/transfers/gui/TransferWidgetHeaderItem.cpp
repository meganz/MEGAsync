#include "TransferWidgetHeaderItem.h"
#include "ui_TransferWidgetHeaderItem.h"

#include "Utilities.h"

TransferWidgetHeaderItem::TransferWidgetHeaderItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferWidgetHeaderItem),
    mCurrentSortOrder(Qt::DescendingOrder)
{
    ui->setupUi(this);

    auto lineSizePolicy = ui->line->sizePolicy();
    lineSizePolicy.setRetainSizeWhenHidden(true);
    ui->line->setSizePolicy(lineSizePolicy);
    ui->line->hide();
}

TransferWidgetHeaderItem::~TransferWidgetHeaderItem()
{
    delete ui;
}

QString TransferWidgetHeaderItem::title() const
{
    return mTitle;
}

void TransferWidgetHeaderItem::setTitle(const QString &title)
{
    mTitle = title;
    ui->columnTitle->setText(title);
}

int TransferWidgetHeaderItem::sortCriterion() const
{
    return mSortCriterion;
}

void TransferWidgetHeaderItem::setSortCriterion(const int &sortCriterion)
{
    mSortCriterion = sortCriterion;
}

void TransferWidgetHeaderItem::setSortOrder(Qt::SortOrder order)
{
    mCurrentSortOrder = order;
    turnOffSiblings();
    updateChevronIcon();
}

void TransferWidgetHeaderItem::turnOffSorting()
{
    mCurrentSortOrder = Qt::DescendingOrder;
    ui->chevron->setVisible(false);
}

void TransferWidgetHeaderItem::forceClick()
{
    auto releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease,QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::NoModifier);
    QApplication::postEvent(this, releaseEvent);
}

void TransferWidgetHeaderItem::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        //If turned off, turn on and turn off siblings
        if(isTurnedOff())
        {
            turnOffSiblings();
        }
        else
        {
            mCurrentSortOrder = mCurrentSortOrder == Qt::DescendingOrder ? Qt::AscendingOrder : Qt::DescendingOrder;
        }

        updateChevronIcon();

        emit toggled(mSortCriterion, mCurrentSortOrder);
    }

    QWidget::mouseReleaseEvent(event);
}

void TransferWidgetHeaderItem::enterEvent(QEvent *event)
{
    ui->line->show();
    QWidget::enterEvent(event);
}

void TransferWidgetHeaderItem::leaveEvent(QEvent *event)
{
    ui->line->hide();
    QWidget::leaveEvent(event);
}

void TransferWidgetHeaderItem::updateChevronIcon()
{
   QIcon icon = mCurrentSortOrder == Qt::DescendingOrder ? Utilities::getCachedPixmap(QString::fromLatin1(":/images/chevron-down-ico.png"))
                                                         : Utilities::getCachedPixmap(QString::fromLatin1(":/images/chevron-up-ico.png"));
   ui->chevron->setPixmap(icon.pixmap(ui->chevron->size()));
   ui->chevron->setVisible(true);
}

void TransferWidgetHeaderItem::turnOffSiblings()
{
    QList<TransferWidgetHeaderItem*> siblings = parentWidget()->findChildren<TransferWidgetHeaderItem *>();

    foreach(auto& headerItem, siblings)
    {
        if(headerItem != this)
        {
            headerItem->turnOffSorting();
        }
    }
}

bool TransferWidgetHeaderItem::isTurnedOff()
{
    return !ui->chevron->isVisible();
}
