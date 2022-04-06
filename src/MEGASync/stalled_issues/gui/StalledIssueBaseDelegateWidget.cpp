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
void StalledIssueBaseDelegateWidget::updateUi(const QModelIndex& index, const QExplicitlySharedDataPointer<StalledIssueData> data)
{
    mCurrentIndex = index;

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

QExplicitlySharedDataPointer<StalledIssueData> StalledIssueBaseDelegateWidget::getData() const
{
    return mData;
}
