#include "StalledIssueBaseDelegateWidget.h"

StalledIssueBaseDelegateWidget::StalledIssueBaseDelegateWidget(QWidget *parent) : QWidget(parent)
{

}

void StalledIssueBaseDelegateWidget::render(const QStyleOptionViewItem &option,
            QPainter *painter,
            const QRegion &sourceRegion)
{
    QWidget::render(painter,QPoint(0,0),sourceRegion);
}
void StalledIssueBaseDelegateWidget::updateUi(const QExplicitlySharedDataPointer<StalledIssueData> data, int)
{
    if(mData != data)
    {
        mData = data;
    }

   refreshUi();
}

QModelIndex StalledIssueBaseDelegateWidget::getCurrentIndex() const
{
    return mCurrentIndex;
}

void StalledIssueBaseDelegateWidget::setCurrentIndex(const QModelIndex &currentIndex)
{
    mCurrentIndex = currentIndex;
}
