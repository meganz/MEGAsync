#include "StalledIssueBaseDelegateWidget.h"

#include "MegaApplication.h"
#include "WordWrapLabel.h"
#include "StalledIssueDelegate.h"

#include "mega/types.h"

#include <QtConcurrent/QtConcurrent>
#include <QFile>

StalledIssueBaseDelegateWidget::StalledIssueBaseDelegateWidget(QWidget *parent)
    : QWidget(parent),
      mDelegate(nullptr)
{
    connect(&mResizeNeedTimer, &QTimer::timeout, this, [this](){
        checkForSizeHintChanges();
    });
    mResizeNeedTimer.setInterval(50);
    mResizeNeedTimer.setSingleShot(true);
}

void StalledIssueBaseDelegateWidget::updateIndex()
{
    if(auto view = dynamic_cast<QWidget*>(parent()))
    {
        view->update();
    }
}

void StalledIssueBaseDelegateWidget::render(const QStyleOptionViewItem &,
            QPainter *painter,
            const QRegion &sourceRegion)
{
    QWidget::render(painter,QPoint(0,0),sourceRegion);
}

void StalledIssueBaseDelegateWidget::updateUi(const QModelIndex& index, const StalledIssueVariant & issueData)
{
    MegaSyncApp->getStalledIssuesModel()->UiItemUpdate(mCurrentIndex, index);

    if(mCurrentIndex != index)
    {
        mCurrentIndex = QPersistentModelIndex(index);
        mData = issueData;
    }

    refreshUi();
}

QModelIndex StalledIssueBaseDelegateWidget::getCurrentIndex() const
{
    return mCurrentIndex;
}

const StalledIssueVariant& StalledIssueBaseDelegateWidget::getData() const
{
    return mData;
}

void StalledIssueBaseDelegateWidget::reset()
{
    mData.reset();
    mCurrentIndex = QModelIndex();
}

QSize StalledIssueBaseDelegateWidget::sizeHint() const
{
    StalledIssue::Type sizeType = isHeader() ? StalledIssue::Header : StalledIssue::Body;

    QSize size;

    if(mData.getDelegateSize(sizeType).isValid())
    {
        size = mData.getDelegateSize(sizeType);
    }

    if(!size.isValid())
    {
        size = QWidget::sizeHint();
        mData.setDelegateSize(size, sizeType);
    }

    return size;
}

bool StalledIssueBaseDelegateWidget::isHeader() const
{
    return mCurrentIndex.isValid() && !mCurrentIndex.parent().isValid();
}

void StalledIssueBaseDelegateWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

bool StalledIssueBaseDelegateWidget::event(QEvent *event)
{
    if(event->type() == QEvent::WhatsThisClicked)
    {
        mResizeNeedTimer.start();
    }

    return QWidget::event(event);
}

void StalledIssueBaseDelegateWidget::checkForSizeHintChanges()
{
    if(mDelegate && mData.consultData())
    {
        layout()->activate();
        updateGeometry();

        mData.removeDelegateSize(StalledIssue::Header);
        mData.removeDelegateSize(StalledIssue::Body);

        //Update sizeHint cache
        sizeHint();

        if(auto stalledDelegate = dynamic_cast<StalledIssueDelegate*>(mDelegate))
        {
            stalledDelegate->updateSizeHint();
        }
    }
}

void StalledIssueBaseDelegateWidget::setDelegate(QStyledItemDelegate *newDelegate)
{
    mDelegate = newDelegate;
}

void StalledIssueBaseDelegateWidget::updateSizeHint()
{
    mResizeNeedTimer.start();
}
