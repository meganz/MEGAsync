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
void StalledIssueBaseDelegateWidget::updateUi(const QModelIndex& index, const StalledIssue &data)
{
    mCurrentIndex = index;
    mData = data;

   refreshUi();
}

QModelIndex StalledIssueBaseDelegateWidget::getCurrentIndex() const
{
    return mCurrentIndex;
}

const StalledIssue& StalledIssueBaseDelegateWidget::getData() const
{
    return mData;
}
