#include "StalledIssueBaseDelegateWidget.h"

#include "MegaApplication.h"

#include "mega/types.h"

#include <QtConcurrent/QtConcurrent>
#include <QFile>

StalledIssueBaseDelegateWidget::StalledIssueBaseDelegateWidget(QWidget *parent)
    : QWidget(parent)
{
    connect(&mUtilities, &StalledIssuesUtilities::actionFinished, this, [this](){
        emit issueFixed();
    });
}

void StalledIssueBaseDelegateWidget::render(const QStyleOptionViewItem &,
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
