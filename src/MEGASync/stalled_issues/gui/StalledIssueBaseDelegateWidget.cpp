#include "StalledIssueBaseDelegateWidget.h"

#include "MegaApplication.h"

#include "mega/types.h"

#include <QtConcurrent/QtConcurrent>
#include <QFile>

StalledIssueBaseDelegateWidget::StalledIssueBaseDelegateWidget(QWidget *parent)
    : QWidget(parent)
{
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
void StalledIssueBaseDelegateWidget::updateUi(const QModelIndex& index, const StalledIssueVariant &data)
{
    mCurrentIndex = QPersistentModelIndex(index);
    mData = data;

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
    StalledIssue::SizeType sizeType = isHeader() ? StalledIssue::Header : StalledIssue::Body;
    if(mData.getDelegateSize(sizeType).isValid())
    {
        return mData.getDelegateSize(sizeType);
    }

    auto size = QWidget::sizeHint();
    mData.setDelegateSize(size, sizeType);

    return size;
}

bool StalledIssueBaseDelegateWidget::isHeader() const
{
    return mCurrentIndex.isValid() && mCurrentIndex.parent().isValid();
}
